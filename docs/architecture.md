# Architecture and data flow

```text
image root
    |
    v
nftw file walk + extension filter
    |
    +--> common identity: size + SHA-256
    |
    +--> OCR      -> Tesseract text
    +--> faces    -> face-detect key/value result
    +--> GPS      -> ExifTool GPS tags
    +--> swatch   -> ImageMagick 1x1 color fingerprint
    |
    v
append-only JSONL report
    |
    v
C localhost duplicate chooser -> decision JSONL
```

`src/photosweep.c` owns the walk, identity record, external-tool boundaries, JSON escaping, report writing, and chooser server. The binary is intentionally small and process-oriented: Tesseract, ExifTool, ImageMagick, and the native face detector can be installed or replaced independently.

## Record contract

All sweep records contain:

```json
{"path":"...","size":123,"sha256":"..."}
```

The sweep adds its own fields and a status. `ok` means the external tool returned usable output; `none` means a valid absence such as no GPS; `unavailable` means the optional tool is not installed; `error` means it was present but failed.

Reports are append-only for resumability. Re-running a sweep appends records, so operators should use a new output directory for a clean production run.

## Boundaries

The analyzer does not organize, move, delete, or edit source images. The old organizer’s behavior is retained as historical documentation and statistics. Any future mutation mode must be a separately named command with an explicit dry-run and manifest.
