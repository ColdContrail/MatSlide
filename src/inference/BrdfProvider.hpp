#pragma once
#include "core/MaterialParameters.hpp"
#include "ONNXRuntimeWrapper.hpp"
#include <vector>
#include <string>

class BrdfProvider {
public:
    bool initialize(const std::string& modelPath);

    bool runInference(const MaterialParameters& params);

    const std::vector<float>& getBrdfData() const { return brdfData; }
    int getPhiHalf() const { return PHI_HALF; }
    int getThetaD() const { return THETA_D; }
    int getThetaH() const { return THETA_H; }
    bool isReady() const { return ready; }

    void setDirtyForConsumer(bool d) { consumerDirty = d; }
    bool isDirtyForConsumer() const { return consumerDirty; }

private:
    static constexpr int THETA_H = 90;
    static constexpr int THETA_D = 90;
    static constexpr int PHI_D = 360;
    static constexpr int PHI_HALF = PHI_D / 2;

    static constexpr double RED_SCALE   = 1.0 / 1500.0;
    static constexpr double GREEN_SCALE = 1.15 / 1500.0;
    static constexpr double BLUE_SCALE  = 1.66 / 1500.0;

    ONNXRuntimeWrapper runtime;
    std::vector<float> brdfData;
    bool ready = false;
    bool consumerDirty = false;
};
