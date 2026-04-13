#include "IblMerlPass.hpp"
#include "geometry.hpp"
#include <memory>
#include <cstdio>
#include <vector>

void IblMerlPass::uploadBrdfTexture() {
    if (!brdfProvider || !brdfProvider->isReady()) {
        fprintf(stderr, "Warning: BRDF data not ready for texture upload\n");
        return;
    }

    const auto& data = brdfProvider->getBrdfData();
    int phiHalf = brdfProvider->getPhiHalf();
    int thetaD = brdfProvider->getThetaD();
    int thetaH = brdfProvider->getThetaH();

    if (brdfTexture) {
        glDeleteTextures(1, &brdfTexture);
        brdfTexture = 0;
    }

    glGenTextures(1, &brdfTexture);
    glBindTexture(GL_TEXTURE_3D, brdfTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F,
                 phiHalf, thetaD, thetaH,
                 0, GL_RGB, GL_FLOAT, data.data());
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void IblMerlPass::setupAccumulationBuffer(int width, int height) {
    screenWidth = width;
    screenHeight = height;

    if (accumTexture) {
        glDeleteTextures(1, &accumTexture);
        accumTexture = 0;
    }
    if (accumFBO) {
        glDeleteFramebuffers(1, &accumFBO);
        accumFBO = 0;
    }
    if (accumDepthRBO) {
        glDeleteRenderbuffers(1, &accumDepthRBO);
        accumDepthRBO = 0;
    }

    glGenTextures(1, &accumTexture);
    glBindTexture(GL_TEXTURE_2D, accumTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
                 width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenRenderbuffers(1, &accumDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, accumDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenFramebuffers(1, &accumFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, accumFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, accumDepthRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    frameCount = 0;
}

void IblMerlPass::setupDisplayQuad() {
    float quadVerts[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };

    glCreateVertexArrays(1, &displayVAO);
    glCreateBuffers(1, &displayVBO);
    glNamedBufferData(displayVBO, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    glEnableVertexArrayAttrib(displayVAO, 0);
    glVertexArrayAttribFormat(displayVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(displayVAO, 0, 0);
    glVertexArrayVertexBuffer(displayVAO, 0, displayVBO, 0, 2 * sizeof(float));
}

void IblMerlPass::init() {
    pass_name = "IBL Merl pass";

    v_mesh.emplace_back(std::make_unique<Mesh>());
    auto loader = std::make_unique<objLoader>();
    loader->parse("assets\\bunny_10k.obj", *(v_mesh.back()));
    if (!v_mesh.back()->set_buffer()) {
        exit(-1);
    }

    brdfShader = std::make_unique<ShaderParser>("src\\shader\\ibl_merl.vert", "src\\shader\\ibl_merl_exp.frag");
    displayShader = std::make_unique<ShaderParser>("src\\shader\\ibl_display.vert", "src\\shader\\ibl_display.frag");

    brdfTexture = 0;
    accumFBO = 0;
    accumTexture = 0;
    accumDepthRBO = 0;
    displayVAO = 0;
    displayVBO = 0;
    frameCount = 0;
    screenWidth = 0;
    screenHeight = 0;

    exrLoader = std::make_unique<ExrLoader>();
    if (!exrLoader->load(exrPath)) {
        fprintf(stderr, "IblMerlPass: failed to load EXR: %s\n", exrPath.c_str());
    }
    if (exrLoader->isLoaded()) {
        exrLoader->uploadCubemap(1024);
    }

    setupAccumulationBuffer(1000, 800);
    setupDisplayQuad();
}

void IblMerlPass::execute() {
    if (brdfProvider && brdfProvider->isReady() &&
        (!brdfTexture || brdfProvider->isDirtyForConsumer())) {
        uploadBrdfTexture();
        brdfProvider->setDirtyForConsumer(false);
        resetAccumulation();
    }

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int vpWidth = viewport[2];
    int vpHeight = viewport[3];
    if (vpWidth != screenWidth || vpHeight != screenHeight) {
        setupAccumulationBuffer(vpWidth, vpHeight);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, accumFBO);
    glViewport(0, 0, screenWidth, screenHeight);

    if (frameCount == 0) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    } else {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glCullFace(GL_BACK);

    brdfShader->use();
    glm::mat4 view       = camera->GetViewMatrix();
    glm::mat4 projection = camera->GetProjectionMatrix(1.25f);

    brdfShader->setProjectionMatrix(projection);
    brdfShader->setViewMatrix(view);
    brdfShader->setModelMatrix(glm::scale(glm::mat4(1.0f), glm::vec3(1.4f)));

    brdfShader->setVec3("viewPos", camera->GetPosition());
    brdfShader->setInt("frameCount", frameCount);

    float roughness = paramManager ? paramManager->getParams().ggxRoughness : 0.1f;
    roughness = std::max(0.05f, roughness);
    brdfShader->setFloat("roughness", roughness);
    if (roughness != prevRoughness) {
        prevRoughness = roughness;
        resetAccumulation();
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, brdfTexture);
    brdfShader->setInt("brdfLUT", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, exrLoader->getCubemapID());
    brdfShader->setInt("envMap", 1);

    glBindVertexArray(v_mesh.back()->get_VAO());

    GLsizei index_count = v_mesh.back()->get_index_count();
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, vpWidth, vpHeight);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    displayShader->use();
    displayShader->setInt("frameCount", frameCount + 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, accumTexture);
    displayShader->setInt("accumResult", 0);

    glBindVertexArray(displayVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

    frameCount++;
}

void IblMerlPass::resetAccumulation() {
    frameCount = 0;
}

void IblMerlPass::clean() {
    brdfShader = nullptr;
    displayShader = nullptr;
    v_mesh.clear();
    exrLoader = nullptr;
    if (brdfTexture) {
        glDeleteTextures(1, &brdfTexture);
        brdfTexture = 0;
    }
    if (accumTexture) {
        glDeleteTextures(1, &accumTexture);
        accumTexture = 0;
    }
    if (accumFBO) {
        glDeleteFramebuffers(1, &accumFBO);
        accumFBO = 0;
    }
    if (accumDepthRBO) {
        glDeleteRenderbuffers(1, &accumDepthRBO);
        accumDepthRBO = 0;
    }
    if (displayVAO) {
        glDeleteVertexArrays(1, &displayVAO);
        displayVAO = 0;
    }
    if (displayVBO) {
        glDeleteBuffers(1, &displayVBO);
        displayVBO = 0;
    }
}

IblMerlPass::IblMerlPass(BrdfProvider* provider, const std::string& path, std::shared_ptr<Camera> cam, ParameterManager* pm)
    : brdfProvider(provider), paramManager(pm), exrPath(path), camera(cam),
      brdfTexture(0), accumFBO(0), accumTexture(0), accumDepthRBO(0),
      displayVAO(0), displayVBO(0),
      frameCount(0), screenWidth(0), screenHeight(0), prevRoughness(pm ? pm->getParams().ggxRoughness : 0.1f) {
    init();
}
