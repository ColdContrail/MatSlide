#include "core/Application.hpp"
#include <rendering/MeshPass.hpp>
#include <memory>

int main() {
    Application app(1000, 800);
    auto p_ms = std::make_unique<MeshPass>();
    app.add_render_pass(std::move(p_ms));
    app.run();
    
    return 0;
}
