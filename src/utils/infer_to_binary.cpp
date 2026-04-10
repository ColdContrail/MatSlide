#include "core/MaterialParameters.hpp"
#include "inference/ONNXRuntimeWrapper.hpp"
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <model.onnx> <output.bin> [10 float values]" << std::endl;
        return 1;
    }

    std::string model_path = argv[1];
    std::string output_path = argv[2];

    MaterialParameters params;
    if (argc >= 13) {
        params.ggxRoughness    = std::stof(argv[3]);
        params.diffuseR        = std::stof(argv[4]);
        params.diffuseG        = std::stof(argv[5]);
        params.diffuseB        = std::stof(argv[6]);
        params.specularR       = std::stof(argv[7]);
        params.specularG       = std::stof(argv[8]);
        params.specularB       = std::stof(argv[9]);
        params.optimalThresholdR = std::stof(argv[10]);
        params.optimalThresholdG = std::stof(argv[11]);
        params.optimalThresholdB = std::stof(argv[12]);
    }

    ONNXRuntimeWrapper runtime;
    if (!runtime.initialize(model_path)) {
        std::cerr << "Failed to initialize ONNX runtime" << std::endl;
        return 1;
    }

    std::vector<float> input_data(params.data(), params.data() + params.size());
    std::vector<float> output_data;

    if (!runtime.runInference(input_data, output_data)) {
        std::cerr << "Inference failed" << std::endl;
        return 1;
    }

    const int C = 3, D1 = 180, D2 = 90, D3 = 90;
    const int oD1 = 90, oD2 = 90, oD3 = 180;

    std::vector<double> permuted(C * oD1 * oD2 * oD3);

    for (int c = 0; c < C; ++c)
        for (int i1 = 0; i1 < D1; ++i1)
            for (int i2 = 0; i2 < D2; ++i2)
                for (int i3 = 0; i3 < D3; ++i3) {
                    int src = ((c * D1 + i1) * D2 + i2) * D3 + i3;
                    int dst = ((c * oD1 + i3) * oD2 + i2) * oD3 + i1;
                    permuted[dst] = static_cast<double>(output_data[src]);
                }

    int32_t dims[3] = {90, 90, 180};
    std::ofstream out(output_path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(dims), sizeof(dims));
    out.write(reinterpret_cast<const char*>(permuted.data()),
              static_cast<std::streamsize>(permuted.size() * sizeof(double)));
    out.close();

    std::cout << "Saved to " << output_path << " (" << permuted.size() << " doubles)" << std::endl;
    return 0;
}