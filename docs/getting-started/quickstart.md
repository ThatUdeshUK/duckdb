# Quickstart

This walks through both flavors of prediction iPDb supports: a classic tabular model, and a prompt-driven LLM
call. Full syntax for everything here is in the [SQL Reference](../reference/sql.md).

## Tabular / GNN models

Use this path when you have a model already trained and exported (e.g. to ONNX) and want to score rows against
it.

1. Load your feature data into a table:

    ```sql
    CREATE TABLE iris AS SELECT * FROM read_csv('iris.csv');
    ```

2. Register the model, telling iPDb which table it runs against and what it outputs:

    ```sql
    CREATE TABULAR MODEL iris_cls PATH 'iris_cls.onnx'
      ON TABLE iris
      OUTPUT (class INTEGER);
    ```

3. Predict:

    ```sql
    SELECT * FROM PREDICT(iris_cls, iris) AS p WHERE p.class = 2;
    ```

`PREDICT` behaves like any other table source — join it, filter it, aggregate over it.

## LLM-backed (semantic) queries

Use this path to run natural-language prompts against rows, instead of (or alongside) plain relational
predicates.

1. Make sure your build has LLM support (`ENABLE_LLM_API=1` for remote models, or `PREDICTOR_IMPL=llama_cpp` for
   local ones — see [Building iPDb](../development/build.md)).

2. If you're calling a remote API, store the credential as a secret (see
   [Configuration](configuration.md) for alternatives):

    ```sql
    CREATE PERSISTENT SECRET openai_key (TYPE http, bearer_token '<your_api_key>');
    ```

3. Register the model:

    ```sql
    CREATE LLM MODEL o4mini PATH 'o4-mini' ON PROMPT
      API 'https://api.openai.com/v1/' SECRET openai_key;
    ```

    `PATH` is whatever model identifier the API expects (`o4-mini`, `gpt-4.1`, ...). For a local model instead,
    point `PATH` at a `.gguf` file and drop `API`/`SECRET`:

    ```sql
    CREATE LLM MODEL gemma2 PATH '/path/to/gemma-2-2b-it.gguf' ON PROMPT;
    ```

4. Query with a prompt. Reference input columns with `{{column}}`, and declare output columns with
   `{column_name TYPE}`:

    ```sql
    SELECT * FROM LLM o4mini (
      PROMPT 'extract the {location VARCHAR} and {salary INTEGER} for job {{description}}' ON jobs
    );
    ```

That's the whole shape: a natural-language instruction, `{{...}}` placeholders for what to read, `{name TYPE}`
placeholders for what to get back.

## Beyond a plain table source

`LLM`/`PREDICT` aren't limited to `FROM` — they compose with the rest of SQL:

```sql
-- as a filter predicate
SELECT p.name, r.review_text
FROM Product AS p JOIN Review AS r ON p.product_id = r.product_id
WHERE LLM o4mini PROMPT 'is the {sentiment VARCHAR} of the {{review_text}} positive or negative' = 'negative';

-- as a join predicate
SELECT p.name
FROM Product AS p
JOIN Product AS o
  ON LLM o4mini PROMPT 'is CPU {{o.name}} {compatible BOOLEAN} with motherboard {{p.name}}';

-- as an aggregate
SELECT category_id, AGG LLM o4mini (PROMPT 'what is the computer {component VARCHAR} from the {{description}}')
FROM Product GROUP BY category_id;
```

See [Working with Input Data](../guides/input-data.md) for shaping data going into these calls, and
[Performance Tuning](../guides/performance-tuning.md) once you're running semantic queries at real volume.

## Next steps

- [Configuration](configuration.md) — secrets, environment variables, and per-model options.
- [SQL Reference](../reference/sql.md) — every `CREATE MODEL`/`PREDICT`/`LLM` form.
- [Execution Model](../concepts/execution-model.md) — how batching and deduplication affect these queries under
  the hood.
