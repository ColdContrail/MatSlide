#include "ONNXRuntimeWrapper.hpp"
#include <iostream>
#include <numeric>

ONNXRuntimeWrapper::ONNXRuntimeWrapper() : initialized(false) {
    try {
        env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "MatSlide");
        session_options = std::make_unique<Ort::SessionOptions>();
        session_options->SetIntraOpNumThreads(1);
        session_options->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
        memory_info = std::make_unique<Ort::MemoryInfo>(
            Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault));
    } catch (const Ort::Exception& e) {
        std::cerr << "ONNX Runtime init error: " << e.what() << std::endl;
    }
}

ONNXRuntimeWrapper::~ONNXRuntimeWrapper() = default;

bool ONNXRuntimeWrapper::initialize(const std::string& model_path) {
    try {
#ifdef _WIN32
        auto wpath = std::wstring(model_path.begin(), model_path.end());
        session = std::make_unique<Ort::Session>(*env, wpath.c_str(), *session_options);
#else
        session = std::make_unique<Ort::Session>(*env, model_path.c_str(), *session_options);
#endif

        Ort::AllocatorWithDefaultOptions allocator;

        auto input_count = session->GetInputCount();
        auto output_count = session->GetOutputCount();

        input_name_strings.resize(input_count);
        output_name_strings.resize(output_count);
        input_names.resize(input_count);
        output_names.resize(output_count);
        input_shapes.resize(input_count);
        output_shapes.resize(output_count);

        for (size_t i = 0; i < input_count; ++i) {
            auto name = session->GetInputNameAllocated(i, allocator);
            input_name_strings[i] = name.get();
            input_names[i] = input_name_strings[i].c_str();
            input_shapes[i] = session->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
        }

        for (size_t i = 0; i < output_count; ++i) {
            auto name = session->GetOutputNameAllocated(i, allocator);
            output_name_strings[i] = name.get();
            output_names[i] = output_name_strings[i].c_str();
            output_shapes[i] = session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
        }

        initialized = true;
        return true;
    } catch (const Ort::Exception& e) {
        std::cerr << "Failed to load ONNX model: " << e.what() << std::endl;
        return false;
    }
}

bool ONNXRuntimeWrapper::runInference(const std::vector<float>& input_data,
                                      std::vector<float>& output_data) {
    if (!initialized || input_names.empty() || output_names.empty()) {
        return false;
    }

    try {
        auto input_shape = input_shapes[0];
        size_t input_tensor_size = std::accumulate(
            input_shape.begin(), input_shape.end(), 1, std::multiplies<int64_t>());

        if (static_cast<size_t>(input_tensor_size) != input_data.size()) {
            std::cerr << "Input size mismatch: expected " << input_tensor_size
                      << ", got " << input_data.size() << std::endl;
            return false;
        }

        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            *memory_info,
            const_cast<float*>(input_data.data()),
            input_data.size(),
            input_shape.data(),
            input_shape.size());

        auto output_shape = output_shapes[0];
        size_t output_tensor_size = std::accumulate(
            output_shape.begin(), output_shape.end(), 1, std::multiplies<int64_t>());

        output_data.resize(output_tensor_size);

        Ort::Value output_tensor = Ort::Value::CreateTensor<float>(
            *memory_info,
            output_data.data(),
            output_data.size(),
            output_shape.data(),
            output_shape.size());

        session->Run(Ort::RunOptions{nullptr},
                     input_names.data(),
                     &input_tensor,
                     1,
                     output_names.data(),
                     &output_tensor,
                     1);

        return true;
    } catch (const Ort::Exception& e) {
        std::cerr << "Inference error: " << e.what() << std::endl;
        return false;
    }
}

std::vector<int64_t> ONNXRuntimeWrapper::getInputShape(size_t index) const {
    if (index < input_shapes.size()) return input_shapes[index];
    return {};
}

std::vector<int64_t> ONNXRuntimeWrapper::getOutputShape(size_t index) const {
    if (index < output_shapes.size()) return output_shapes[index];
    return {};
}

size_t ONNXRuntimeWrapper::getInputCount() const { return input_names.size(); }
size_t ONNXRuntimeWrapper::getOutputCount() const { return output_names.size(); }
