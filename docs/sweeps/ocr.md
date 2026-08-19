# OCR sweep

Command:

```sh
bin/photosweep run ocr PHOTO_ROOT REPORT_DIR
```

The C runner invokes `tesseract IMAGE stdout -l eng` once per supported image. The returned text is JSON-escaped into `ocr.jsonl`; source images and EXIF are untouched.

## Statuses

- `ok`: Tesseract completed; `text` contains its output.
- `unavailable`: Tesseract is not on `PATH`; `text` is empty.
- `error`: Tesseract ran but returned a failure.

The workstation’s Recoll service and OCR cache are documented in the archive notes. They are intentionally not copied into this repository because indexes, caches, and home paths are machine-specific.
