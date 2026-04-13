#include "core/Application.hpp"
#include <rendering/EnvMapPass.hpp>
#include <rendering/MerlPass.hpp>
#include <rendering/IblMerlPass.hpp>
#include <inference/BrdfProvider.hpp>
#include <core/Camera.hpp>
#include <memory>

int main() {
    Application app(1000, 800);

    auto brdfProvider = std::make_unique<BrdfProvider>();
    if (!brdfProvider->initialize("assets\\onnx\\decoder.onnx")) {
        std::cerr << "Failed to initialize BRDF provider" << std::endl;
        return 1;
    }

    brdfProvider->runInference(app.getParamManager().getParams());

    BrdfProvider* providerPtr = brdfProvider.get();

    auto sharedCamera = std::make_shared<Camera>();
    sharedCamera -> Rotate(10.0f, -45.0f);

    auto p_env = std::make_unique<EnvMapPass>("assets\\env\\envmap.exr", sharedCamera);
    app.add_render_pass(std::move(p_env));

    // auto p_ms = std::make_unique<MerlPass>(providerPtr, sharedCamera);
    // app.add_render_pass(std::move(p_ms));

    auto p_ibl = std::make_unique<IblMerlPass>(providerPtr, "assets\\env\\envmap.exr", sharedCamera, &app.getParamManager());
    app.add_render_pass(std::move(p_ibl));

    app.getParamManager().subscribe([providerPtr](const MaterialParameters& params) {
        if (providerPtr->runInference(params)) {
            providerPtr->setDirtyForConsumer(true);
        }
    });

    app.run();

    return 0;
}
