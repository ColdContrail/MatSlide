#include "BrdfProvider.hpp"
#include <iostream>

bool BrdfProvider::initialize(const std::string& modelPath) {
    if (!runtime.initialize(modelPath)) {
        std::cerr << "BrdfProvider: Failed to initialize ONNX runtime" << std::endl;
        return false;
    }
    return true;
}

bool BrdfProvider::runInference(const MaterialParameters& params) {
    if (!runtime.isInitialized()) {
        std::cerr << "BrdfProvider: ONNX runtime not initialized" << std::endl;
        return false;
    }

    std::vector<float> inputData(params.data(), params.data() + params.size());
    std::vector<float> outputData;

    if (!runtime.runInference(inputData, outputData)) {
        std::cerr << "BrdfProvider: Inference failed" << std::endl;
        return false;
    }

    const int C = 3, D1 = 180, D2 = 90, D3 = 90;
    const int oD1 = 90, oD2 = 90, oD3 = 180;

    int totalSize = C * oD1 * oD2 * oD3;
    std::vector<double> permuted(totalSize);

    for (int c = 0; c < C; ++c)
        for (int i1 = 0; i1 < D1; ++i1)
            for (int i2 = 0; i2 < D2; ++i2)
                for (int i3 = 0; i3 < D3; ++i3) {
                    int src = ((c * D1 + i1) * D2 + i2) * D3 + i3;
                    int dst = ((c * oD1 + i3) * oD2 + i2) * oD3 + i1;
                    permuted[dst] = static_cast<double>(outputData[src]);
                }

    int n = THETA_H * THETA_D * PHI_HALF;
    brdfData.resize(PHI_HALF * THETA_D * THETA_H * 3);

    for (int ith = 0; ith < THETA_H; ith++) {
        for (int itd = 0; itd < THETA_D; itd++) {
            for (int ipd = 0; ipd < PHI_HALF; ipd++) {
                int merlIdx = ipd + itd * PHI_HALF + ith * PHI_HALF * THETA_D;
                int texIdx = (ith * THETA_D * PHI_HALF + itd * PHI_HALF + ipd) * 3;

                double scales[3] = {RED_SCALE, GREEN_SCALE, BLUE_SCALE};
                for (int c = 0; c < 3; ++c) {
                    brdfData[texIdx + c] = static_cast<float>(permuted[merlIdx + c * n] * scales[c]);
                }
            }
        }
    }

    ready = true;
    return true;
}
