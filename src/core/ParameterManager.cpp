#include "ParameterManager.hpp"
#include <imgui.h>

MaterialParameters& ParameterManager::getParams() {
    return params;
}

const MaterialParameters& ParameterManager::getParams() const {
    return params;
}

void ParameterManager::setDirty(bool d) {
    dirty = d;
}

bool ParameterManager::isDirty() const {
    return dirty;
}

void ParameterManager::subscribe(Callback cb) {
    subscribers.emplace_back(std::move(cb));
}

void ParameterManager::notify() {
    for (auto& cb : subscribers) {
        cb(params);
    }
    dirty = false;
}

void ParameterManager::renderGUI() {
    ImGui::Begin("Material Parameters");

    if (ImGui::SliderFloat("GGX Roughness", &params.ggxRoughness, 0.0f, 1.0f)) dirty = true;
    ImGui::Separator();

    ImGui::Text("Diffuse Albedo");
    if (ImGui::SliderFloat("Diffuse R", &params.diffuseR, 0.0f, 0.5f)) dirty = true;
    if (ImGui::SliderFloat("Diffuse G", &params.diffuseG, 0.0f, 0.5f)) dirty = true;
    if (ImGui::SliderFloat("Diffuse B", &params.diffuseB, 0.0f, 0.5f)) dirty = true;
    ImGui::Separator();

    ImGui::Text("Specular Albedo");
    if (ImGui::SliderFloat("Specular R", &params.specularR, 0.0f, 0.5f)) dirty = true;
    if (ImGui::SliderFloat("Specular G", &params.specularG, 0.0f, 0.5f)) dirty = true;
    if (ImGui::SliderFloat("Specular B", &params.specularB, 0.0f, 0.5f)) dirty = true;
    ImGui::Separator();

    ImGui::Text("Optimal Threshold");
    if (ImGui::SliderFloat("Threshold R", &params.optimalThresholdR, 0.0f, 1.0f)) dirty = true;
    if (ImGui::SliderFloat("Threshold G", &params.optimalThresholdG, 0.0f, 1.0f)) dirty = true;
    if (ImGui::SliderFloat("Threshold B", &params.optimalThresholdB, 0.0f, 1.0f)) dirty = true;

    ImGui::End();
}
