# PhotoSort / PhotoSweep

PhotoSweep is a native C photo-analysis and review toolkit descended from the original PhotoSort PHP keyboard sorter. The legacy application is preserved at [`archive/legacy-photosort/`](archive/legacy-photosort/).

## Start here

```sh
make clean all
make check
bin/photosweep run all /path/to/photo-library /tmp/photosweep-demo/reports
bin/photosweep chooser /tmp/photosweep-demo/reports/swatch.jsonl
```

Open the printed localhost URL to review the report. Analysis is read-only: it does not move files, delete duplicates, rewrite EXIF, or upload GPS data.

## Documentation map

| Need | Read |
|---|---|
| Five-minute overview | [`docs/README.md`](docs/README.md) |
| Architecture and data flow | [`docs/architecture.md`](docs/architecture.md) |
| OCR sweep | [`docs/sweeps/ocr.md`](docs/sweeps/ocr.md) |
| Face sweep | [`docs/sweeps/faces.md`](docs/sweeps/faces.md) |
| GPS/GIS sweep | [`docs/sweeps/gps.md`](docs/sweeps/gps.md) |
| Swatch and duplicates | [`docs/sweeps/swatch.md`](docs/sweeps/swatch.md) |
| Duplicate chooser | [`docs/duplicate-review.md`](docs/duplicate-review.md) |
| Production demo | [`docs/production-demo.md`](docs/production-demo.md) |
| Operations and privacy | [`docs/operations.md`](docs/operations.md) |
| Function-by-function call path | [`docs/function-reference.md`](docs/function-reference.md) |
| C development | [`docs/development.md`](docs/development.md) |
| System inventory | [`docs/system-inventory.md`](docs/system-inventory.md) |
| Rules and thresholds | [`rules/photosweep.yaml`](rules/photosweep.yaml) |

## Commands

```text
bin/photosweep run ocr    ROOT OUTDIR
bin/photosweep run faces  ROOT OUTDIR
bin/photosweep run gps    ROOT OUTDIR
bin/photosweep run swatch ROOT OUTDIR
bin/photosweep run all    ROOT OUTDIR
bin/photosweep chooser    REPORT [PORT]
```

Each sweep appends one JSON object per image. Every record includes the path, byte size, and SHA-256 identity. Optional tools produce an explicit `unavailable` or `error` status instead of silently inventing results.

## Design promises

- C is the only active implementation language.
- Original files are inputs, never implicit outputs.
- Exact duplicates are identified by SHA-256; similarity is a review signal.
- GPS and face results stay local unless an operator deliberately exports them.
- Historical machine notes and non-portable caches are documentation/archive material, not production inputs.
