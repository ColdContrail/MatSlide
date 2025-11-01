#include "MeshPass.hpp"
#include "geometry.hpp"
#include <memory>

void MeshPass::init() {
    pass_name = "Mesh pass";

    v_mesh.emplace_back(std::make_unique<Mesh>());
    auto loader = std::make_unique<objLoader>();
    loader->parse("assets\\bunny_10k.obj", *(v_mesh.back()));
    if (!v_mesh.back()->set_buffer()) {
        exit(-1);
    }

    parser = std::make_unique<ShaderParser>("src\\shader\\v1.vert", "src\\shader\\v1.frag");

    camera = std::make_unique<Camera>();
}

void MeshPass::execute() {

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    auto& shader_parser = parser;

    shader_parser->use();
    

    camera -> Rotate(0.0f, 0.01f);
    glm::mat4 view       = camera->GetViewMatrix();
    glm::mat4 projection = camera->GetProjectionMatrix(1.25f);

    shader_parser -> setProjectionMatrix(projection);
    shader_parser -> setViewMatrix(view);
    shader_parser -> setModelMatrix(glm::mat4(1.4f));

    glBindVertexArray(v_mesh.back()->get_VAO());

    GLsizei index_count = v_mesh.back()->get_index_count();
    GLsizei vertex_count = v_mesh.back()->get_vertex_count();
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void MeshPass::clean() {
    parser = nullptr;
    v_mesh.clear();
}

MeshPass::MeshPass() {
    init();
}