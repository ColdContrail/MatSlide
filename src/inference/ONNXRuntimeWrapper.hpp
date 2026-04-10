#pragma once
#include <onnxruntime_cxx_api.h>
#include <vector>
#include <string>
#include <memory>

class ONNXRuntimeWrapper {
private:
    std::unique_ptr<Ort::Env> env;
    std::unique_ptr<Ort::Session> session;
    std::unique_ptr<Ort::SessionOptions> session_options;
    std::unique_ptr<Ort::MemoryInfo> memory_info;

    std::vector<std::string> input_name_strings;
    std::vector<std::string> output_name_strings;
    std::vector<const char*> input_names;
    std::vector<const char*> output_names;
    std::vector<std::vector<int64_t>> input_shapes;
    std::vector<std::vector<int64_t>> output_shapes;

    bool initialized;

public:
    ONNXRuntimeWrapper();
    ~ONNXRuntimeWrapper();

    bool initialize(const std::string& model_path);
    bool runInference(const std::vector<float>& input_data,
                      std::vector<float>& output_data);

    std::vector<int64_t> getInputShape(size_t index = 0) const;
    std::vector<int64_t> getOutputShape(size_t index = 0) const;
    size_t getInputCount() const;
    size_t getOutputCount() const;
    bool isInitialized() const { return initialized; }
};
