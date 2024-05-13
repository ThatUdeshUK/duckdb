#include "duckdb_torch.hpp"
#include <chrono>
#include <algorithm>

namespace duckdb {
TorchPredictor::TorchPredictor() : Predictor() {}

void TorchPredictor::Load(const std::string &model_path, PredictStats &stats) {
    try {
#if OPT_TIMING
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif
        model = torch::jit::load(model_path.c_str());
#if OPT_TIMING
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        stats.load = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
//        std::cout << "Load @run: " << stats.load << std::endl;
#endif
    } catch (const c10::Error &e) {
        error_message = "error loading the model\n";
    }
}

void TorchPredictor::Predict(std::vector<float> &input, std::vector<float> &output, int output_size) {
    std::vector <torch::jit::IValue> inputs;
    auto options = torch::TensorOptions().dtype(torch::kFloat32);
    auto my_input = torch::from_blob(input.data(), {1, (int) input.size()}, options);
    inputs.push_back(my_input);

    at::Tensor out = model.forward(inputs).toTensor();
    output.push_back(*out.data_ptr<float>());
}

void TorchPredictor::PredictVector(std::vector<float> &input, std::vector<float> &output, int m, int n,
                                   int output_size) {
    std::vector <torch::jit::IValue> inputs;
    auto options = torch::TensorOptions().dtype(torch::kFloat32);
    auto my_input = torch::from_blob(input.data(), {m, n}, options);
    inputs.push_back(my_input);

    at::Tensor out = model.forward(inputs).toTensor();

//    auto out = my_output.slice(/*dim=*/1, /*start=*/0, /*end=*/output_size);
//    std::vector<float> v(out.data_ptr<float>(), out.data_ptr<float>() + out.numel());

//    torch::Tensor my_mean = at::mean(my_input);

    output.insert(output.end(), out.data_ptr<float>(), out.data_ptr<float>() + out.numel());
}

void TorchPredictor::PredictChunk(DataChunk &input, DataChunk &output, int m, int n, int output_size,
                                  PredictStats &stats) {
#if OPT_TIMING
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif

    std::vector <torch::jit::IValue> inputs;
    auto options = torch::TensorOptions().dtype(torch::kFloat32);

    std::vector <torch::Tensor> my_input_lists;
    for (int i = 0; i < n; ++i) {
        my_input_lists.push_back(torch::from_blob(input.data[i].GetData(), {m, 1}, options));
    }
    torch::Tensor my_input = at::cat(my_input_lists, 1);
    inputs.push_back(my_input);

#if OPT_TIMING
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    stats.move += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
//    std::cout << "Move @run: " << stats.move << std::endl;

    begin = std::chrono::steady_clock::now();
#endif
    at::Tensor out = model.forward(inputs).toTensor();
#if OPT_TIMING
    end = std::chrono::steady_clock::now();
    stats.predict += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
//    std::cout << "Predict @run: " << stats.predict << std::endl;

    begin = std::chrono::steady_clock::now();
#endif
#if MOVE_REV_METHOD == ROW_FIRST_SET
    idx_t idx = 0;
    for (auto i = out.data_ptr<float>(); i != out.data_ptr<float>() + out.numel(); ++i) {
        output.SetValue(idx % output_size, idx / output_size, Value(*i));
        idx++;
    }
#endif
#if MOVE_REV_METHOD == COL_FIRST_SET
    float* data_ptr = out.data_ptr<float>();
    for (idx_t idx = 0; idx < output_size; idx++) {
        for (idx_t data_i = 0; data_i < m; ++data_i) {
            float* data = data_ptr + idx + (data_i * output_size);
            output.SetValue(idx, data_i, Value(*data));
        }
    }
#endif
#if MOVE_REV_METHOD == ROW_FIRST_COPY
    idx_t idx = 0;
    for (auto i = out.data_ptr<float>(); i != out.data_ptr<float>() + out.numel(); ++i) {
        auto dest = output.data[idx % output_size].GetData();
        *((float*) dest + (idx / output_size)) = *i;
        idx++;
    }
#endif
#if MOVE_REV_METHOD == COL_FIRST_COPY
    for (idx_t idx = 0; idx < output_size; idx++) {
        float* float_pointer = out.data_ptr<float>() + idx;
        data_ptr_t start = data_ptr_cast(float_pointer);

        auto dest = output.data[idx].GetData();
        for (int i = 0; i < m; ++i) {
            *((float*) dest + i) = *((float*) start + i * output_size);
        }
    }
#endif
#if OPT_TIMING
    end = std::chrono::steady_clock::now();
    stats.move_rev += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
//    std::cout << "Move Rev @run: " << stats.move_rev << std::endl;
#endif
}
} // namespace imdb
