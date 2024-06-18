#pragma once

#include <string>
#include "torch/script.h" // One-stop header.
#include <memory>
#include "duckdb/common/types/data_chunk.hpp"
#include "duckdb/common/types/vector.hpp"
#include "duckdb/execution/operator/projection/physical_predict.hpp"

#define ROW_FIRST_SET 0
#define COL_FIRST_SET 1
#define ROW_FIRST_COPY 2
#define COL_FIRST_COPY 3
#define MOVE_REV_METHOD COL_FIRST_COPY
#define OPT_TIMING 1

namespace duckdb {
class TorchPredictor: public Predictor {
public:
    TorchPredictor();

public:
    void Load(const std::string &model_path, PredictStats &stats) override;
    void Predict(std::vector<float> &input, std::vector<float> &output, int output_size) override;
    void PredictVector(std::vector<float> &input, std::vector<float> &output, int m, int n, int output_size) override;
    void PredictChunk(DataChunk &input, DataChunk &output, int rows, const std::vector<idx_t> &input_mask, int output_size, PredictStats &stats) override;
private:
    torch::jit::script::Module model;
};
} // namespace duckdb
