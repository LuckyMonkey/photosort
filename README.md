# PhotoSort / PhotoSweep 📸

PhotoSweep is a native C photo-analysis and review toolkit descended from the original PhotoSort PHP keyboard sorter. PhotoSort depends on the companion [Csharp](https://github.com/LuckyMonkey/Csharp) repository when HEIC/HEIF files need safe JPEG derivatives; they remain separate repositories and binaries: Csharp converts, PhotoSweep analyzes and reviews. The legacy application is preserved at [`archive/legacy-photosort/`](archive/legacy-photosort/).

## Start here 🚀

```sh
make clean all
make check
bin/photosweep run all /path/to/photo-library /tmp/photosweep-demo/reports
bin/photosweep chooser /tmp/photosweep-demo/reports/swatch.jsonl
```

Open the printed localhost URL to review the report. Analysis is read-only: it does not move files, delete duplicates, rewrite EXIF, or upload GPS data.

### The complete pipeline

```text
📥 library → Csharp (optional HEIC/HEIF → JPEG)
          → PhotoSweep inventory + SHA-256 identity
          → 🔤 OCR · 🙂 faces · 📍 GPS · 🎨 swatch · 🖼️ raster passes
          → append-only JSONL/TSV reports
          → 🎨 swatch/dHash candidate report
          → 🖥️ comparison tool: human keep/skip/delete decision
          → optional, explicit organization/cleanup
```

Csharp preserves the HEIF input, selects direct NVDEC only for supported opaque 8-bit 4:2:0 images, and falls back to CPU decoding for unsupported formats. PhotoSweep applies narrow, inspectable filters rather than one opaque classifier.

## Documentation map

| Need | Read |
|---|---|
| Five-minute overview | [`docs/README.md`](docs/README.md) |
| Architecture and data flow | [`docs/architecture.md`](docs/architecture.md) |
| OCR sweep | [`docs/sweeps/ocr.md`](docs/sweeps/ocr.md) |
| Face sweep | [`docs/sweeps/faces.md`](docs/sweeps/faces.md) |
| Planned face identity warm-up | [`docs/sweeps/face-identity.md`](docs/sweeps/face-identity.md) |
| GPS/GIS sweep | [`docs/sweeps/gps.md`](docs/sweeps/gps.md) |
| Swatch and duplicates | [`docs/sweeps/swatch.md`](docs/sweeps/swatch.md) |
| Duplicate chooser / comparison tool | [`docs/duplicate-review.md`](docs/duplicate-review.md) |
| Production demo | [`docs/production-demo.md`](docs/production-demo.md) |
| Operations and privacy | [`docs/operations.md`](docs/operations.md) |
| Function-by-function call path | [`docs/function-reference.md`](docs/function-reference.md) |
| C development | [`docs/development.md`](docs/development.md) |
| System inventory | [`docs/system-inventory.md`](docs/system-inventory.md) |
| Rules and thresholds | [`rules/photosweep.yaml`](rules/photosweep.yaml) |
| Csharp integration | [`docs/integration-csharp.md`](docs/integration-csharp.md) |
| Importing source libraries | [`docs/import-sources.md`](docs/import-sources.md) |

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

## Filter methods 🔎

- **File filter:** walks regular files and ignores unsupported extensions.
- **Identity filter:** uses SHA-256 to prove exact byte duplicates.
- **Metadata filter:** extracts GPS, camera/date evidence, and dimensions without changing the source.
- **OCR filter:** sends eligible images to Tesseract and records text or a clear status.
- **Face filter:** records face presence/count; it does not claim identity.
- **Visual filter:** uses swatch/dHash similarity to create review candidates. Similarity is never automatic deletion.
- **Raster filter:** classifies dimensions and likely screenshots using configured rules.

Each pass is independent, resumable, and append-only. A missing optional dependency becomes `unavailable`; a tool failure becomes `error`.

## Comparison tool 🖥️

The visual report is not a deletion command. Open the local comparison tool with the generated swatch report, inspect each pair side by side, and decide what to keep:

- `A` keeps the left image and quarantines the right.
- `D` keeps the right image and quarantines the left.
- `W` keeps both.
- `S` skips the pair.
- `X` deletes both into the review quarantine.
- `Z` undoes the most recent decision.

The tool records decisions in TSV and moves rejected files into a review-quarantine directory rather than silently deleting them. Empty that quarantine only after reviewing the decision report and confirming backups. See [`docs/duplicate-review.md`](docs/duplicate-review.md).

## Design promises

- C is the only active implementation language.
- Original files are inputs, never implicit outputs.
- Exact duplicates are identified by SHA-256; similarity is a review signal.
- GPS and face results stay local unless an operator deliberately exports them.
- Historical machine notes and non-portable caches are documentation/archive material, not production inputs.
