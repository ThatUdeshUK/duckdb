# Running iPDb

## Interactive shell

iPDb's CLI is the standard DuckDB shell, renamed `ipdb`, with the prediction/LLM SQL surface available whenever
the binary was built with it enabled (see [Building iPDb](../development/build.md)).

```bash
./ipdb                 # in-memory database
./ipdb mydata.duckdb    # persistent database file
```

Everything you already know about the DuckDB shell works as-is: dot-commands (`.mode`, `.headers`, `.output`,
`.tables`, ...), multi-line statements terminated with `;`, `.read <file>` to execute a script, `.help` for the
full command list. See the [Reference](../reference/cli.md) for the flags and dot-commands most relevant to iPDb
workflows.

## Running a script non-interactively

```bash
./ipdb mydata.duckdb -c "SELECT * FROM ipdb_models();"
./ipdb mydata.duckdb < queries.sql
```

`demo.sql` at the repository root is a good end-to-end example script covering model creation, secrets,
semantic projections/selections/joins/aggregates, and relevant `SET` options — a useful reference while you're
still learning the syntax.

## From Python

If you installed the [Python package](../getting-started/installation.md#python-package), the API is the
[DuckDB Python client](https://duckdb.org/docs/stable/clients/python/overview) unchanged, plus the SQL additions
documented here:

```python
import ipdb

con = ipdb.connect("mydata.duckdb")
con.sql("CREATE PERSISTENT SECRET openai_key (TYPE http, bearer_token '...')")
con.sql("CREATE LLM MODEL o4mini PATH 'o4-mini' ON PROMPT API 'https://api.openai.com/v1/' SECRET openai_key")
result = con.sql("""
    SELECT * FROM LLM o4mini (PROMPT 'extract the {location VARCHAR} for job {{description}}' ON jobs)
""").df()
```

## A typical session

1. Load or attach your data (`CREATE TABLE ... AS SELECT * FROM read_csv(...)`, `ATTACH`, etc. — see
   [Working with Input Data](input-data.md)).
2. Register whichever models you need (`CREATE MODEL` / `CREATE LLM MODEL` / `CREATE EMBEDDING` — see
   [Quickstart](../getting-started/quickstart.md) and [Configuration](../getting-started/configuration.md)).
3. Query normally, using `PREDICT`/`LLM` wherever you'd otherwise need an external inference step.
4. Tune batching/deduplication settings if you're running the same shape of query repeatedly at volume — see
   [Performance Tuning](performance-tuning.md).

## Checking what's registered

```sql
SELECT * FROM duckdb_secrets();     -- configured API credentials
SELECT * FROM ipdb_models();        -- registered models
SELECT * FROM ipdb_embeddings();    -- registered embeddings
```
