# PhotoSort / PhotoSweep

PhotoSort began as a small PHP keyboard sorter: show one image, expose its EXIF, and move it to a chosen folder with a key press. That original snapshot is preserved in [`archive/legacy-photosort/`](archive/legacy-photosort/).

PhotoSweep builds on that idea for large libraries. It separates analysis from action:

```text
bin/photosweep        C analyzer: EXIF/date/camera classification, safe moves, hashes
bin/photosweep run    four composable analysis passes and JSONL reports
bin/photosweep chooser local browser review of duplicate groups
rules/                editable classification and safety policy
```

## Four sweep types

| Sweep | Result | Default behavior |
|---|---|---|
| `ocr` | searchable text per image | calls Tesseract when installed; never edits originals |
| `faces` | face presence/count | calls the configured detector when available; otherwise records `unavailable` |
| `gps` | coordinates and optional readable location | reads GPS EXIF; no network geocoding by default |
| `swatch` | color fingerprint and perceptual duplicate groups | groups likely duplicates for review; never deletes automatically |

Every pass writes appendable JSONL. The analysis binary does not move files or write metadata. Analysis reports can be inspected by the duplicate chooser:

```bash
make
 bin/photosweep run all /path/to/photos reports
bin/photosweep chooser reports/swatch.jsonl
```

The historical shell organizer is preserved under [`archive/non-c-active/`](archive/non-c-active/). The active analyzer is C and uses ExifTool, SHA-256, Tesseract, the face detector, and ImageMagick when installed.

## Safety rules

- Original pixels and embedded EXIF are not rewritten by analysis passes.
- Exact duplicates are identified by SHA-256; similar files are suggestions only.
- GPS is private data: it is reported locally and never sent to a service by default.
- Reports are line-oriented and resumable; interrupted runs can be repeated.
- Review decisions are recorded separately from source files.

See [`rules/photosweep.yaml`](rules/photosweep.yaml) for the policy and [`docs/workflow.md`](docs/workflow.md) for the operating model.
