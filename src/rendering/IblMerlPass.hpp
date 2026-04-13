#pragma once

#include "RenderPass.hpp"
#include <glad/glad.h>
#include <core/geometry.hpp>
#include <core/ObjLoader.hpp>
#include <core/ShaderParser.hpp>
#include <core/Camera.hpp>
#include <core/ExrLoader.hpp>
#include <core/ParameterManager.hpp>
#include <inference/BrdfProvider.hpp>
#include <memory>
#include <vector>
#include <string>

class IblMerlPass : public RenderPass {
private:
    std::vector<std::unique_ptr<Mesh>> v_mesh;
    std::unique_ptr<ShaderParser> brdfShader;
    std::unique_ptr<ShaderParser> displayShader;
    std::shared_ptr<Camera> camera;

    GLuint brdfTexture;
    BrdfProvider* brdfProvider;
    ParameterManager* paramManager;
    float prevRoughness;

    std::unique_ptr<ExrLoader> exrLoader;
    std::string exrPath;

    GLuint accumFBO;
    GLuint accumTexture;
    GLuint accumDepthRBO;
    GLuint displayVAO;
    GLuint displayVBO;
    int frameCount;
    int screenWidth;
    int screenHeight;

    void uploadBrdfTexture();
    void setupAccumulationBuffer(int width, int height);
    void setupDisplayQuad();

public:
    IblMerlPass(BrdfProvider* provider, const std::string& exrPath, std::shared_ptr<Camera> cam, ParameterManager* pm);
    virtual void init();
    virtual void execute();
    virtual void clean();

    void resetAccumulation();
};
