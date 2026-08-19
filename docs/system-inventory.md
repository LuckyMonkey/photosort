# System inventory

The repository now contains the portable parts of the photo systems developed on the workstation:

| System | Native entry point | Output | State |
|---|---|---|---|
| EXIF/hash inventory | `bin/photosweep run gps` plus common identity fields | JSONL | included |
| OCR | `bin/photosweep run ocr` -> Tesseract | JSONL text | included |
| face detection | `bin/photosweep run faces` -> `face-detect` adapter | JSONL count/presence | included as external native hook |
| GPS/GIS | `bin/photosweep run gps` -> ExifTool | local JSONL coordinates | included; geocoding off by default |
| swatch/exact duplicate signal | `bin/photosweep run swatch` plus SHA-256 | JSONL fingerprint/hash | included |
| duplicate review | `bin/photosweep chooser` | localhost review server and decision log | included |
| legacy keyboard sorter | `archive/legacy-photosort/` | historical PHP snapshot | archived |

Machine-specific Recoll indexes, OCR caches, face caches, GPS exports, and photo-library reports are deliberately not committed. The archived notes document their behavior and the C entry point replaces the Python glue in the active project.

All active source code in this repository is C. External tools are process-level integrations with explicit unavailable/error states.
