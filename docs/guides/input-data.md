# Working with Input Data

Loading data into iPDb is unchanged from DuckDB — CSV/Parquet/JSON readers, `ATTACH`, `COPY`, and the Python/other
client APIs all work as usual. This guide focuses on what's specific to feeding data into `PREDICT`/`LLM`.

## Loading data (standard DuckDB)

```sql
CREATE TABLE jobs AS SELECT * FROM read_csv('jobs.csv');
CREATE TABLE reviews AS SELECT * FROM read_parquet('reviews.parquet');
```

See [DuckDB's data import documentation](https://duckdb.org/docs/stable/data/overview) for the full set of
readers and options — nothing here is iPDb-specific.

## What `PREDICT`/`LLM` can take as input

Both `PREDICT` and `LLM` (as a table source) accept anything that produces rows: a table, a subquery, or a join
result.

```sql
-- a plain table
SELECT * FROM PREDICT(iris_cls, iris) AS p;

-- a subquery, so you can shape/pre-filter the input first
SELECT title, genre, main_character
FROM LLM llama3 (PROMPT 'extract the {genre VARCHAR} and {main_character VARCHAR} from the {{plot}}' ON (
    SELECT title, mi.info AS plot
    FROM movie m
    JOIN movie_info mi ON m.id = mi.movie_id
    JOIN info_type it ON mi.info_type_id = it.id
    WHERE it.info = 'plot'
));
```

Used as a scalar expression instead (in a filter, join predicate, or `GROUP BY`), `LLM`/`PREDICT` read whichever
columns their prompt/model references from whatever's already in scope at that point in the query — see
[Quickstart](../getting-started/quickstart.md) for the different call shapes.

## Matching columns to a tabular/GNN model

A `CREATE TABULAR MODEL`/`CREATE GNN MODEL` is bound to a specific input table shape (see
[Quickstart](../getting-started/quickstart.md#tabular-gnn-models)) — the columns iPDb feeds the model at
inference time are whatever that table exposes, so make sure the table `PREDICT` reads from has the same feature
columns the model was trained/exported with, in a form the backend (ONNX or your GNN format) expects.

## Preparing data for LLM prompts

For prompt-based (`LLM`) calls, there's no fixed input schema — you write the column list directly in the prompt
with `{{column}}` placeholders. This means:

- Any column referenced in the prompt must be present (and named the same) in the input relation.
- If the natural column name isn't what you want the model to see, alias it in the subquery you pass in (as
  `mi.info AS plot` does above) rather than trying to rename inside the prompt.
- Keep the input text reasonably clean — trimming boilerplate/noise from a text column before it reaches the
  prompt tends to help both output quality and semantic deduplication (see
  [Execution Model](../concepts/execution-model.md#semantic-deduplication-clustering)), since dedup groups rows by
  similarity of exactly the text you hand it.

## Embeddings

`CREATE EMBEDDING` computes an embedding for a table/column once, up front, using a registered `EMBED` model, so
later semantic queries and clustering over that column don't pay to re-embed it every time:

```sql
CREATE EMBED MODEL text_embed PATH 'text-embedding-3-small' API 'https://api.openai.com/v1/' SECRET openai_key;
CREATE EMBEDDING review_embed ON reviews (review_text) USING text_embed;
```

Once created, embeddings are visible via the `ipdb_embeddings()` table function, and are reused automatically by
semantic deduplication (see [Execution Model](../concepts/execution-model.md#embeddings)) without any change to
your query text.

## Next steps

Once your data's loaded, head to [Quickstart](../getting-started/quickstart.md) to register a model and run your
first prediction query, or [Performance Tuning](performance-tuning.md) if you're already running semantic queries
at volume.
