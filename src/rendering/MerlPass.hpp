#include "RenderPass.hpp"
#include <glad/glad.h>
#include <core/geometry.hpp>
#include <core/ObjLoader.hpp>
#include <core/ShaderParser.hpp>
#include <core/Camera.hpp>
#include <inference/BrdfProvider.hpp>
#include <memory>
#include <vector>

class MerlPass : public RenderPass {
private:
    std::vector<std::unique_ptr<Mesh>> v_mesh;
    std::unique_ptr<ShaderParser> parser;
    std::shared_ptr<Camera> camera;
    GLuint brdfTexture;

    glm::vec3 lightPos;
    float lightRotationAngle;
    float lightRotationRadius;
    float lightRotationSpeed;

    BrdfProvider* brdfProvider;

    void uploadBrdfTexture();
public:
    MerlPass(BrdfProvider* provider, std::shared_ptr<Camera> cam);
    virtual void init();
    virtual void execute();
    virtual void clean();
};
