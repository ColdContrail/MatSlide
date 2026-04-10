#include "RenderPass.hpp"
#include <glad/glad.h>
#include <core/geometry.hpp>
#include <core/ObjLoader.hpp>
#include <core/ShaderParser.hpp>
#include <core/Camera.hpp>
#include <memory>
#include <vector>
#include <string>

class MerlPass : public RenderPass {
private:
    std::vector<std::unique_ptr<Mesh>> v_mesh;
    std::unique_ptr<ShaderParser> parser;
    std::unique_ptr<Camera> camera;
    GLuint brdfTexture;

    glm::vec3 lightPos;
    float lightRotationAngle;
    float lightRotationRadius;
    float lightRotationSpeed;

    bool loadMerlBRDF(const std::string& filename);
public:
    MerlPass();
    virtual void init();
    virtual void execute();
    virtual void clean();
};