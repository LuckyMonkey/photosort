# Production demo runbook

This is the short path for demonstrating the complete native pipeline without touching the originals.

```sh
make clean all
make check
bin/photosweep run all /path/to/photo-library /tmp/photosweep-demo/reports
bin/photosweep chooser /tmp/photosweep-demo/reports/swatch.jsonl 8765
```

Open `http://127.0.0.1:8765/` locally. The chooser is review-only; it does not move or delete files. Stop it with `Ctrl-C`.

## Preflight

The C binary is always built. Individual enrichment tools are optional and are reported as `unavailable` rather than guessed:

```sh
command -v exiftool       # GPS and embedded metadata
command -v tesseract      # OCR
command -v face-detect    # native face detector adapter
command -v convert        # swatch fingerprints
```

Use a copy or read-only mount for the first demo. Keep GPS reports private. The production organizer/move action is intentionally separate from these report-only sweeps and requires a deliberate follow-up implementation decision.
