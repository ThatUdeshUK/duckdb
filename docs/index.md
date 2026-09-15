---
title: iPDb
---

# iPDb

**iPDb** is a database engine, built on [DuckDB](https://duckdb.org), that adds a native relational
**prediction operator** to SQL. It lets you run inference — against tabular/GNN models, local LLMs, or remote
LLM APIs — directly inside your queries, right alongside joins, filters, and aggregates.

```sql
-- semantic SQL: predict with a large language model
SELECT p.name, r.review_text
FROM Product AS p
JOIN Review AS r ON p.product_id = r.product_id
WHERE LLM o4mini PROMPT 'is the {sentiment VARCHAR} of the {{review_text}} positive or negative' = 'negative';
```

iPDb is the reference implementation for the paper
["iPDB -- Optimizing SQL Queries with ML and LLM Predicates"](https://arxiv.org/abs/2601.16432).

## Why iPDb

- **Inference as a first-class relational operator.** `PREDICT` and `LLM` behave like any other SQL construct —
  they can appear as a table source, a scalar expression, a join predicate, or a `GROUP BY` key — and compose with
  the rest of the query.
- **Query-aware execution.** Because the optimizer understands `PREDICT`/`LLM` calls, it can push filters through
  them, avoid running inference on rows that a `LIMIT` will discard, and reorder joins around expensive inference
  calls. See [Execution Model](concepts/execution-model.md).
- **Pluggable backends.** Run tabular/GNN models via ONNX, local LLMs via llama.cpp, or any OpenAI-compatible
  remote LLM API — all through the same SQL surface.
- **It's still DuckDB.** Everything you already know about DuckDB (the CLI, SQL dialect, Python/other client APIs,
  file formats) works unchanged. iPDb only adds to it.

## Where to go next

- **New to iPDb?** Start with [Installation](getting-started/installation.md) and the
  [Quickstart](getting-started/quickstart.md).
- **Setting up models or API keys?** See [Configuration](getting-started/configuration.md).
- **Writing queries against your own data?** See [Working with Input Data](guides/input-data.md).
- **Curious how it works under the hood?** See [Architecture](concepts/architecture.md) and
  [Execution Model](concepts/execution-model.md).
- **Looking for a specific `SET` option or SQL statement?** See the [Reference](reference/cli.md) section.
- **Want to build iPDb from source or contribute?** See [Development](development/build.md).

## Project links

- Paper: [arXiv:2601.16432](https://arxiv.org/abs/2601.16432)
- Source: [github.com/purduedb/iPDb](https://github.com/purduedb/iPDb)
- Upstream: [DuckDB](https://github.com/duckdb/duckdb) — iPDb tracks DuckDB's own documentation for everything
  that isn't specific to the prediction operator.

!!! note "Citing iPDb"
    ```bibtex
    @misc{ipdb2026arxiv,
          title={iPDB -- Optimizing SQL Queries with ML and LLM Predicates},
          author={Udesh Kumarasinghe and Tyler Liu and Chunwei Liu and Walid G. Aref},
          year={2026},
          eprint={2601.16432},
          archivePrefix={arXiv},
          primaryClass={cs.DB},
          url={https://arxiv.org/abs/2601.16432},
    }
    ```
