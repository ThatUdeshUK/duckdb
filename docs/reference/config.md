# Settings

Set with `SET <name> = <value>;` in the shell or over any client connection; read back with
`SELECT current_setting('<name>');` or `.settings`-style introspection, same as any other DuckDB setting. These
are in addition to DuckDB's own [configuration options](https://duckdb.org/docs/stable/configuration/overview).

## LLM inference

| Setting | Type | Default | Meaning |
|---|---|---|---|
| `llm_use_batch` | `BOOLEAN` | `true` | Batch multiple rows into a single LLM call instead of one call per row. |
| `ml_batch_size` | `UBIGINT` | `16` | Rows per batched inference call (LLM and ONNX). |
| `llm_use_cache` | `BOOLEAN` | `true` | Enable/disable exact tuple deduplication: rows with an exactly matching (model, prompt, input) are sent to the LLM once and share that result, within a single query execution. Not a cross-query result cache despite the name. |
| `llm_max_tokens` | `UBIGINT` | `512` | Maximum output tokens per LLM call. |
| `llm_no_threads` | `UBIGINT` | `16` | Thread count used for local (llama.cpp) LLM inference. |
| `pull_predict_filter` | `BOOLEAN` | `true` | Enable pulling/pushing filters around `PREDICT`/`LLM` operators so inference doesn't run on rows a filter would discard. Disable only to isolate a suspected optimizer bug. |

## ONNX inference

| Setting | Type | Default | Meaning |
|---|---|---|---|
| `onnx_execution_mode` | `UBIGINT` | `0` | ONNX Runtime execution mode: `0` = sequential, `1` = parallel. |
| `onnx_intra_tc` | `UBIGINT` | `16` | ONNX Runtime intra-op thread count. |
| `onnx_inter_tc` | `UBIGINT` | `512` | ONNX Runtime inter-op thread count. |

## Model auto-selection

Used when an `LLM`/`AGG LLM` call omits a model name and more than one candidate model exists in the catalog. If a
model name is given explicitly, none of this runs — that exact model is used (or an error is raised if it doesn't
exist).

| Setting | Type | Default | Meaning |
|---|---|---|---|
| `model_select_strategy` | `VARCHAR` | `first` | `first`, `random`, or `optimal`. `optimal` requires a build with `ENABLE_PREDICT`; in a build without it, `optimal` silently behaves like `first`. |
| `model_select_quality_weight` | `DOUBLE` | `1.0` | `optimal` only: weight on a candidate's MMLU-Pro quality score (higher = prefer quality). |
| `model_select_cost_weight` | `DOUBLE` | `0.0` | `optimal` only: weight on cost efficiency (higher = prefer cheaper models). |
| `model_select_time_weight` | `DOUBLE` | `0.0` | `optimal` only: weight on latency (higher = prefer faster models). |
| `model_select_min_quality` | `DOUBLE` | `0.0` | `optimal` only: minimum acceptable MMLU-Pro score (0–100); `0` disables the constraint. |
| `model_select_max_cost` | `DOUBLE` | `0.0` | `optimal` only: maximum USD per output token; `0` disables the constraint. |
| `model_select_max_time` | `DOUBLE` | `0.0` | `optimal` only: maximum seconds per output token; `0` disables the constraint. |

```sql
SET model_select_strategy = 'optimal';
SET model_select_quality_weight = 0.7;
SET model_select_cost_weight = 0.3;
```

## Per-model `OPTIONS`

Most of the settings above can also be overridden **per model**, via the `OPTIONS {...}` clause on
`CREATE LLM MODEL` / `CREATE TABULAR MODEL` / `CREATE GNN MODEL` / `CREATE EMBED MODEL` (see
[SQL Reference](sql.md)). An option set on the model takes precedence over the session-wide setting for calls to
that model only.

| Option key | Applies to | Falls back to setting |
|---|---|---|
| `batch_size` | LLM, ONNX | `ml_batch_size` |
| `llm_max_tokens` | LLM, ONNX | `llm_max_tokens` |
| `use_cache` | LLM | `llm_use_cache` — exact tuple deduplication (see [Execution Model](../concepts/execution-model.md#exact-tuple-deduplication)) |
| `use_batch` | LLM | `llm_use_batch` |
| `n_threads` | LLM | `llm_no_threads` |
| `req_per_min` | LLM (remote API) | none — defaults to `500` if omitted |
| `onnx_execution_mode` | ONNX | `onnx_execution_mode` |
| `onnx_intra_tc` | ONNX | `onnx_intra_tc` |
| `onnx_inter_tc` | ONNX | `onnx_inter_tc` |

```sql
CREATE LLM MODEL o4mini PATH 'o4-mini' ON PROMPT
  API 'https://api.openai.com/v1/' SECRET openai_key
  OPTIONS {"req_per_min": 30, "batch_size": 32};
```

`req_per_min` is the main lever for staying under a remote vendor's rate limit — see
[Performance Tuning](../guides/performance-tuning.md).
