# PhotoSort + Csharp integration ⚙️📸

PhotoSort and Csharp are separate GitHub repositories with a dependency relationship:

| Repository | Responsibility |
|---|---|
| [PhotoSort / PhotoSweep](https://github.com/LuckyMonkey/photosort) | Inventory, OCR, faces, GPS, raster classification, visual similarity, and review |
| [Csharp](https://github.com/LuckyMonkey/Csharp) | Safe HEIC/HEIF → JPEG conversion when a JPEG derivative is needed |

They are not merged into one binary or Git history. Csharp is an optional conversion dependency; PhotoSweep remains useful for libraries that already contain readable images.

## Recommended order

1. Preserve the source and work from a backup when possible.
2. Run Csharp for HEIC/HEIF files that need JPEG derivatives. Inputs stay untouched.
3. Run PhotoSweep against the selected source or derivative tree.
4. Open the PhotoSort comparison tool and review swatch/dHash candidates side by side; similarity is a signal, not proof.
5. Apply organization or cleanup only as a separate, explicit operation.

## How the filters work 🔎

PhotoSweep uses narrow filters: regular-file/extension filtering, SHA-256 identity, metadata extraction, OCR, face presence/count, GPS, raster dimensions, and visual swatch/dHash similarity. Every pass writes append-only JSONL with explicit `ok`, `none`, `unavailable`, or `error` status.

Csharp uses container/capability gates for bit depth, chroma, alpha, auxiliary images, grids, HDR, and malformed input. It uses direct NVDEC only for the supported fast path and falls back to libheif CPU decoding otherwise. JPEG output is written atomically with supported metadata and orientation handling.

## Smoke test

```sh
cmake -S /path/to/Csharp -B /tmp/csharp-build
cmake --build /tmp/csharp-build -j
/tmp/csharp-build/csharp --backend cpu sample.heic /tmp/sample.jpg

cd /path/to/photosort
make clean all
make check
bin/photosweep run all /tmp/sample-library /tmp/photosweep-reports
```

Do not publish generated GPS, face, OCR, or library reports; they may contain private locations, text, or paths.


## Comparison tool decision gate 🖥️

PhotoSweep’s comparison tool consumes the visual swatch report and serves a local-only browser UI. `A` keeps left, `D` keeps right, `W` keeps both, `S` skips, `X` quarantines both, and `Z` undoes the last decision. Decisions are appended to a TSV report; rejected files are moved to a review quarantine first. This keeps automated similarity detection separate from the irreversible cleanup decision.
