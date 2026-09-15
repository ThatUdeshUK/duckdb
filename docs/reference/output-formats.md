# Output Formats

iPDb is a normal DuckDB engine for output purposes — every standard DuckDB output format, client result type, and
export mechanism works unchanged. This page covers just the shell-level basics plus what's specific to
`PREDICT`/`LLM` result typing.

## Shell output modes

Set with the `.mode` dot-command in the `ipdb` shell:

```
.mode duckbox   -- default: pretty-printed box table (interactive)
.mode csv
.mode json
.mode line
.mode markdown
.mode insert
```

Redirect output to a file with `.output <file>` (and `.output` with no argument to return to stdout). See
`.help` in the shell, or
[DuckDB's CLI documentation](https://duckdb.org/docs/stable/clients/cli/output_formats), for the complete list —
none of this is changed by iPDb.

## Exporting query results

Standard `COPY`, unchanged:

```sql
COPY (SELECT * FROM PREDICT(iris_cls, iris)) TO 'output.parquet' (FORMAT parquet);
COPY (SELECT * FROM LLM o4mini (PROMPT '...' ON jobs)) TO 'output.csv' (HEADER, DELIMITER ',');
```

From a client library (Python, etc.), pull results into whatever native structure you'd normally use
(`.df()`, `.arrow()`, `.fetchall()`, ...) — see the
[DuckDB Python client docs](https://duckdb.org/docs/stable/clients/python/overview).

## Typed output from `PREDICT`/`LLM`

Prediction results are ordinary typed columns, not opaque blobs — the column types come from:

- The model's declared `OUTPUT` columns, for `CREATE MODEL`/`CREATE TABULAR MODEL`.
- The type annotation on each output placeholder in the prompt, for `LLM ... PROMPT ...` calls (e.g.
  `{summary VARCHAR}`, `{gone_down BOOLEAN}`, `{state_tax DOUBLE}`) — see [SQL Reference](sql.md#prompt-placeholders)
  for the full placeholder syntax.

Because the result is typed like any other column, it composes normally with everything downstream: `WHERE`,
`GROUP BY`, joins, casts, and every output format above.

## Inspecting registered models and embeddings

```sql
SELECT * FROM ipdb_models();
SELECT * FROM ipdb_embeddings();
```

return one row per registered model/embedding — see the [CLI Reference](cli.md#table-functions) for the full
column lists.
