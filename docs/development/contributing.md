# Contributing

iPDb follows DuckDB's own contribution process and code style unchanged. See
[`CONTRIBUTING.md`](https://github.com/purduedb/iPDb/blob/main/CONTRIBUTING.md) at the repo root for the full
guidelines (issue/PR workflow, code style, formatting) and the
[Code of Conduct](https://github.com/purduedb/iPDb/blob/main/CODE_OF_CONDUCT.md).

The one iPDb-specific note: if your change touches the prediction/LLM extension, prefer adding real sqllogictest
coverage under `test/sql/` rather than relying on manual `demo.sql`-style verification — see
[Testing](testing.md).
