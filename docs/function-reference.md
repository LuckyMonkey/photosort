# Function-by-function reference

This page follows one image through the C program. The goal is to make the code readable during a demo and make the safety boundaries obvious.

## Shared path

```text
main
  -> is_supported_image
  -> walk_callback
  -> sweep_file
       -> shell_quote
       -> write_common_identity
            -> run_command(sha256sum)
       -> run_command(exiftool | tesseract | face-detect | convert)
       -> write_json_string
```

### `main(argc, argv)`

Selects either `run` or `chooser`, validates the required arguments, creates the output directory, and loops over the requested sweep names. The `all` request is expanded here so each pass receives its own report file. Keeping dispatch in one place makes the four modes explicit and prevents a failed optional tool from changing another mode.

### `is_supported_image(path)`

Checks the extension against the policy list. It is a cheap early filter so videos, JSON sidecars, and unrelated files never reach image tools.

### `walk_callback(path, stat, type, state)`

Receives callbacks from `nftw`. It forwards regular supported image files to `sweep_file`. `FTW_PHYS` avoids following symlinks, which prevents accidental traversal outside the chosen library.

### `sweep_file(path)`

Owns the per-sweep decision. It first writes the common identity fields, then invokes exactly one external analyzer based on `mode`, maps the result to a status, and closes one JSON object. It never calls `rename`, `unlink`, or an EXIF-writing tool.

### `write_common_identity(report, path, hash)`

Computes the file’s SHA-256 through `sha256sum` and records path, byte size, and hash. The hash is the stable identity used for exact duplicate review; filenames and paths are not reliable identities.

### `run_command(command, buffer, capacity)`

Runs a bounded external command through `popen`, captures stdout, NUL-terminates it, and returns success/failure. The caller converts failure into an explicit JSON status instead of aborting the whole library run.

### `shell_quote(destination, capacity, path)`

Quotes a path for the external command boundary, including embedded single quotes. This is needed because photo libraries commonly contain spaces, parentheses, and punctuation.

### `write_json_string(report, text)`

Escapes quotes, backslashes, and newlines before writing a JSON string. OCR text and tool output are user data, so they cannot be printed raw into JSON.

## Sweep-specific call paths

### OCR

```text
sweep_file
  -> write_common_identity
  -> run_command("tesseract IMAGE stdout -l eng")
  -> write_json_string(text)
```

Why: Tesseract is already a well-tested native OCR engine. The repository stores its text in a report rather than changing the image or creating a sidecar.

### Faces

```text
sweep_file
  -> write_common_identity
  -> run_command("face-detect IMAGE")
  -> write_json_string(key/value detector output)
```

Why: the detector is an optional native hook. A missing detector must be visible as `unavailable`, because absence of a result is not evidence that an image has no faces.

### GPS/GIS

```text
sweep_file
  -> write_common_identity
  -> run_command("exiftool -s3 -n GPS tags")
  -> write_json_string(coordinates)
```

Why: ExifTool handles camera-specific EXIF layouts. The first stage stops at local coordinate extraction; geocoding is a separate privacy-sensitive decision.

### Swatch and exact duplicates

```text
sweep_file
  -> write_common_identity              # SHA-256 exact identity
  -> run_command("convert IMAGE -resize 1x1!")
  -> write_json_string(color)
```

Why: a color swatch is useful for grouping candidates but is not proof of duplication. SHA-256 is the exact signal; the chooser is the human review boundary for anything approximate.

## Chooser path

```text
serve_chooser(report, port)
  -> socket/bind/listen on 127.0.0.1
  -> accept request
  -> escaped report response
  -> POST /decision -> duplicate-decisions.jsonl
```

Why: the chooser is intentionally local and review-only. It records a decision but does not interpret that decision as permission to delete or move a file.
