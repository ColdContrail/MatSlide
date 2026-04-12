#include "EnvMapPass.hpp"

EnvMapPass::EnvMapPass(const std::string& exrPath, std::shared_ptr<Camera> cam)
    : camera(cam), skyboxVAO(0), skyboxVBO(0) {
    exrLoader = std::make_unique<ExrLoader>();
    if (!exrLoader->load(exrPath)) {
        fprintf(stderr, "EnvMapPass: failed to load EXR: %s\n", exrPath.c_str());
    }
    init();
}

void EnvMapPass::setupSkybox() {
    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glCreateVertexArrays(1, &skyboxVAO);
    glCreateBuffers(1, &skyboxVBO);

    glNamedBufferData(skyboxVBO, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexArrayAttrib(skyboxVAO, 0);
    glVertexArrayAttribFormat(skyboxVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(skyboxVAO, 0, 0);
    glVertexArrayVertexBuffer(skyboxVAO, 0, skyboxVBO, 0, 3 * sizeof(float));
}

void EnvMapPass::init() {
    pass_name = "EnvMap pass";
    parser = std::make_unique<ShaderParser>("src\\shader\\envmap.vert", "src\\shader\\envmap.frag");
    setupSkybox();

    if (exrLoader->isLoaded()) {
        exrLoader->uploadCubemap(1024);
    }
}

void EnvMapPass::execute() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    parser->use();

    glm::mat4 view = camera->GetRotationMatrix();
    glm::mat4 projection = camera->GetProjectionMatrix(1.25f);
    parser->setViewMatrix(view);
    parser->setProjectionMatrix(projection);

    glBindTextureUnit(0, exrLoader->getCubemapID());
    parser->setInt("envMap", 0);

    glBindVertexArray(skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

void EnvMapPass::clean() {
    parser = nullptr;
    exrLoader = nullptr;
    camera = nullptr;
    if (skyboxVAO) {
        glDeleteVertexArrays(1, &skyboxVAO);
        skyboxVAO = 0;
    }
    if (skyboxVBO) {
        glDeleteBuffers(1, &skyboxVBO);
        skyboxVBO = 0;
    }
}
