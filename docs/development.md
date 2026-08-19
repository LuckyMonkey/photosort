# C development

## Build and test

```sh
make clean all
make check
git diff --check
```

The Makefile uses C11, optimization, and strict warnings. `make check` verifies CLI usage, runs all four sweeps over the archived three-image fixture, and checks that each report has three records.

## Adding a sweep

Keep the sweep inside the C dispatch in `src/photosweep.c` and document it under `docs/sweeps/`. It must:

1. reuse the common path/size/SHA-256 record;
2. escape output as valid JSON;
3. emit `ok`, `none`, `unavailable`, or `error`;
4. avoid source mutation;
5. add a deterministic fixture assertion to `Makefile` or a future C test harness.

Do not add Python, PHP, shell, Perl, machine caches, home-directory paths, or private photo reports to the active implementation. Historical material belongs under `archive/` and must be clearly labeled.
