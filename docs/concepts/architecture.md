# Architecture

iPDb is DuckDB with a prediction/LLM extension layered onto the standard pipeline — parsing, binding/planning,
optimization, and execution. If you already know how DuckDB executes a query, the mental model below is mostly
"where does `PREDICT`/`LLM` slot in," not a new pipeline to learn.

```
SQL text
   │
   ▼
Parser            CREATE MODEL / CREATE LLM MODEL / CREATE EMBEDDING / PREDICT(...) / LLM ... PROMPT ...
   │               are recognized as first-class statements and expressions.
   ▼
Binder / Planner  Model/prompt references are resolved against the catalog; a logical PREDICT operator
   │               is planned like any other relational operator (scan, join, filter, ...).
   ▼
Optimizer         Filters, limits, and joins around PREDICT/LLM calls are rewritten to avoid running
   │               inference on rows that don't need it (inference is the expensive part of the plan).
   ▼
Executor          The physical prediction operator dispatches to a pluggable backend based on model type.
   │
   ▼
Model backend     ONNX (tabular/GNN) · llama.cpp (local LLM) · remote LLM API (OpenAI-compatible)
```

## Catalog: models and embeddings

Models and embeddings are catalog objects, just like tables and views:

- A **model**, created via `CREATE MODEL` / `CREATE TABULAR MODEL` / `CREATE LLM MODEL`, records where the model
  lives (a local file path or a remote API), its input/output columns (or how to infer them from a prompt), and
  any backend-specific options.
- An **embedding**, created via `CREATE EMBEDDING`, attaches embedding metadata to a table column so later
  semantic queries can reuse it without recomputing.

Because models are catalog entries, they persist with the database and can be listed/inspected with SQL — see
`ipdb_models()` in the [Reference](../reference/cli.md).

## The prediction operator

`PREDICT` and `LLM` can appear anywhere a normal relational construct can: as a table source
(`FROM PREDICT(model, table)`), as a scalar expression in a filter or projection, as a join predicate, or as a
`GROUP BY`/aggregate key. The planner treats inference as a first-class operator rather than a black-box UDF,
which is what lets the optimizer reason about it.

## Optimizing around inference

Because model/LLM inference is orders of magnitude more expensive than a normal relational operator, the
optimizer includes passes specifically aimed at reducing how much inference actually runs:

- **Filter pushdown around PREDICT** — filters that don't depend on the prediction's own output can be pushed
  below it, so inference only runs on rows that survive the filter.
- **Limit-aware inference** — a `LIMIT` on top of a filtered prediction can be pushed in front of the prediction
  itself, so inference stops once enough matching rows are found.
- **Join-aware predicate extraction** — when a filter spans a join and references a prediction, the plan is
  restructured so the prediction's result becomes an ordinary filterable column, keeping the join reorderer free
  to pick a good join order instead of being blocked by the prediction.
- **Cost-aware join ordering** — the join planner accounts for the cost of any `PREDICT`/`LLM` calls in the plan
  when choosing a join order, rather than assuming all operators are equally cheap.

See [Execution Model](execution-model.md) for how this plays out for LLM-backed queries specifically (batching,
deduplication, model auto-selection).

## Model backends

The same `PREDICT`/`LLM` SQL surface is served by different backends depending on the model type and how iPDb was
built (see [Building iPDb](../development/build.md)):

| Model type | Backend | Typical use |
|---|---|---|
| Tabular / GNN | ONNX Runtime | Pre-trained models exported to `.onnx` |
| Local LLM | llama.cpp | Local inference on GGUF-format models |
| Remote LLM | HTTP (OpenAI-compatible) | Any vendor exposing an OpenAI-style chat/completions API |

Only the backends you build with are compiled in — a build without `PREDICTOR_IMPL=llama_cpp`, for instance,
won't have local-LLM support even if the SQL syntax accepts a local model path.
