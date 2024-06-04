#pragma once

#include <onnxruntime_cxx_api.h>

#include <string>
#include <memory>
#include <map>
#include "duckdb/common/types/data_chunk.hpp"
#include "duckdb/common/types/vector.hpp"
#include "duckdb/execution/operator/projection/physical_predict.hpp"
#include <tokenizer.hpp>

#define ROW_FIRST_SET 0
#define COL_FIRST_SET 1
#define ROW_FIRST_COPY 2
#define COL_FIRST_COPY 3
#define ROW_FIRST_PUSH 4

#define MOVE_METHOD COL_FIRST_COPY
#define MOVE_REV_METHOD COL_FIRST_COPY

#define OPT_TIMING 1

namespace duckdb {
class ONNXPredictor: public Predictor {
public:
    ONNXPredictor();

public:
    void Load(const std::string &model_path, PredictStats &stats) override;
    void Predict(std::vector<float> &input, std::vector<float> &output, int output_size) override;
    void PredictLM(std::string &input, std::vector<float> &output, int output_size) override;
    void PredictLMChunk(DataChunk &input, DataChunk &output, int rows, int output_size, PredictStats &stats) override;
    void PredictChunk(DataChunk &input, DataChunk &output, int rows, int cols, int output_size, PredictStats &stats) override;
private:
    Ort::Session session{nullptr};
    FullTokenizer tokenizer;

    void Preprocess(const std::string &text, long* input_ids, long* mask, int offset, int max_length) const;
};
} // namespace duckdb
