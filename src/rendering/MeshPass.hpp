#include "RenderPass.hpp"
#include <glad/glad.h>
#include <core/geometry.hpp>
#include <core/ObjLoader.hpp>
#include <core/ShaderParser.hpp>
#include <core/Camera.hpp>
#include <memory>
#include <vector>

class MeshPass : public RenderPass {
private:
    std::vector<std::unique_ptr<Mesh>> v_mesh;
    std::unique_ptr<ShaderParser> parser;
    std::unique_ptr<Camera> camera;
public:
    MeshPass();
    virtual void init();
    virtual void execute();
    virtual void clean();
};