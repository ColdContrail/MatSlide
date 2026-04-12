#pragma once

#include "RenderPass.hpp"
#include <glad/glad.h>
#include <core/ExrLoader.hpp>
#include <core/ShaderParser.hpp>
#include <core/Camera.hpp>
#include <memory>

class EnvMapPass : public RenderPass {
private:
    std::unique_ptr<ExrLoader> exrLoader;
    std::unique_ptr<ShaderParser> parser;
    std::shared_ptr<Camera> camera;
    GLuint skyboxVAO;
    GLuint skyboxVBO;

    void setupSkybox();

public:
    EnvMapPass(const std::string& exrPath, std::shared_ptr<Camera> cam);
    virtual void init();
    virtual void execute();
    virtual void clean();
};
