#include "duckdb_onnx.hpp"
#include <chrono>
#include <algorithm>
#include <iostream>

namespace duckdb {
ONNXPredictor::ONNXPredictor() : Predictor() {}

void ONNXPredictor::Load(const std::string &model_path, PredictStats &stats) {
#if OPT_TIMING
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif
    std::string instanceName{"model"};
    static Ort::Env env(OrtLoggingLevel::ORT_LOGGING_LEVEL_WARNING, instanceName.c_str());

    Ort::SessionOptions sessionOptions;
    sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    Ort::Session session_tmp(env, model_path.c_str(), sessionOptions);
    session = std::move(session_tmp);
#if OPT_TIMING
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    stats.load = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
//    std::cout << "Load @run: " << stats.load << std::endl;
#endif
}

void ONNXPredictor::Predict(std::vector<float> &input, std::vector<float> &output, int output_size) {

    std::array<int64_t, 2> input_shape_{1, static_cast<int>(input.size())};
    std::array<int64_t, 2> output_shape_{1, output_size};

    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    Ort::Value input_tensor_ = Ort::Value::CreateTensor<float>(memory_info, input.data(), input.size(),
                                                               input_shape_.data(), input_shape_.size());

    const char* input_names[] = {"input"};
    const char* output_names[] = {"output"};

    Ort::RunOptions run_options;
    auto output_tensor_ = session.Run(run_options, input_names, &input_tensor_, 1, output_names, 1);

    float* floatarr = output_tensor_[0].GetTensorMutableData<float>();

    output.insert(output.end(), floatarr, floatarr + output_size);
}

void ONNXPredictor::PredictChunk(DataChunk &input, DataChunk &output, int m, int n, int output_size,
                                  PredictStats &stats) {
#if OPT_TIMING
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif

    // TODO: Create tensors
    std::array<int64_t, 2> input_shape_{m, n};
    std::array<int64_t, 2> output_shape_{m, output_size};

    std::vector<float> input_data;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            input_data.push_back(*((float*) (input.data[j].GetData()) + i));
        }
    }
    std::vector<float> output_data;
    output_data.reserve(m * output_size);
    for (int i = 0; i < m * output_size; ++i) {
        output_data.emplace_back(0);
    }

    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    Ort::Value input_tensor_ = Ort::Value::CreateTensor<float>(memory_info, input_data.data(), input_data.size(),
                                                               input_shape_.data(), input_shape_.size());
    Ort::Value output_tensor_ = Ort::Value::CreateTensor<float>(memory_info, output_data.data(), output_data.size(),
                                                               output_shape_.data(), output_shape_.size());

    const char* input_names[] = {"input"};
    const char* output_names[] = {"output"};

#if OPT_TIMING
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    stats.move += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

    begin = std::chrono::steady_clock::now();
#endif

    Ort::RunOptions run_options;
    session.Run(run_options, input_names, &input_tensor_, 1, output_names, &output_tensor_, 1);

#if OPT_TIMING
    end = std::chrono::steady_clock::now();
    stats.predict += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

    begin = std::chrono::steady_clock::now();
#endif

    auto out_data_ptr = output_data.data();
    for (idx_t idx = 0; idx < output_size; idx++) {
        float* float_pointer = out_data_ptr + idx;
        data_ptr_t start = data_ptr_cast(float_pointer);

        auto dest = output.data[idx].GetData();
        for (int i = 0; i < m; ++i) {
            *((float*) dest + i) = *((float*) start + i * output_size);
        }
    }

#if OPT_TIMING
    end = std::chrono::steady_clock::now();
    stats.move_rev += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
#endif
}
} // namespace duckdb
