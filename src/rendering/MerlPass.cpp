#include "MerlPass.hpp"
#include "geometry.hpp"
#include <memory>
#include <cstdio>
#include <vector>

void MerlPass::uploadBrdfTexture() {
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

void MerlPass::init() {
    pass_name = "Merl pass";

    v_mesh.emplace_back(std::make_unique<Mesh>());
    auto loader = std::make_unique<objLoader>();
    loader->parse("assets\\bunny_10k.obj", *(v_mesh.back()));
    if (!v_mesh.back()->set_buffer()) {
        exit(-1);
    }

    parser = std::make_unique<ShaderParser>("src\\shader\\merl.vert", "src\\shader\\merl.frag");

    lightPos = glm::vec3(2.0f, 3.0f, 1.0f);
    lightRotationAngle = 0.0f;
    lightRotationRadius = 3.0f;
    lightRotationSpeed = 0.1f;

    brdfTexture = 0;
}

void MerlPass::execute() {

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (brdfProvider && brdfProvider->isReady() &&
        (!brdfTexture || brdfProvider->isDirtyForConsumer())) {
        uploadBrdfTexture();
        brdfProvider->setDirtyForConsumer(false);
    }

    parser->use();

    // camera->Rotate(0.7f, -1.0f);
    glm::mat4 view       = camera->GetViewMatrix();
    glm::mat4 projection = camera->GetProjectionMatrix(1.25f);

    parser->setProjectionMatrix(projection);
    parser->setViewMatrix(view);
    parser->setModelMatrix(glm::mat4(1.4f));

    lightRotationAngle += lightRotationSpeed * 0.016f;
    lightPos.x = glm::cos(lightRotationAngle) * lightRotationRadius;
    lightPos.z = glm::sin(lightRotationAngle) * lightRotationRadius;
    lightPos.y = 3.0f;

    parser->setVec3("lightPos", lightPos);
    parser->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    parser->setFloat("lightIntensity", 100.0f);
    parser->setVec3("viewPos", camera->GetPosition());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, brdfTexture);
    parser->setInt("brdfLUT", 0);

    glBindVertexArray(v_mesh.back()->get_VAO());

    GLsizei index_count = v_mesh.back()->get_index_count();
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void MerlPass::clean() {
    parser = nullptr;
    v_mesh.clear();
    if (brdfTexture) {
        glDeleteTextures(1, &brdfTexture);
        brdfTexture = 0;
    }
}

MerlPass::MerlPass(BrdfProvider* provider, std::shared_ptr<Camera> cam)
    : brdfProvider(provider), camera(cam) {
    init();
}
