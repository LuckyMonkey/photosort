# PhotoSort documentation 📚

The root README gets you oriented; this directory is **level 2**: practical guides for actually importing, inspecting, repairing, reviewing, and operating a photo library.

If you are new, do not read every page in order. Pick the question you are trying to answer.

## I have a pile of photos. What do I do first?

1. **Get the originals safely onto disk:** [Import pipelines](import-sources.md)
2. **Understand what the files know about themselves:** [What is EXIF?](exif.md)
3. **Understand PhotoSort's stages:** [Workflow](workflow.md)
4. **Run a small test library first:** [Production demo](production-demo.md)
5. **Review possible duplicates yourself:** [Duplicate review](duplicate-review.md)

> **Rule zero:** import first, verify second, mutate later. A 90,000-photo library is a terrible place to discover that `sync` meant “make both sides identical.” 😬

## Operator guides

| Question | Guide |
|---|---|
| How do I get photos off a phone, camera, SD card, cloud service, or old disk? | [Import pipelines](import-sources.md) |
| What are EXIF, IPTC, XMP, timestamps, GPS, ISO, shutter speed, and aperture? | [What is EXIF?](exif.md) |
| How does the complete pipeline fit together? | [Workflow](workflow.md) |
| How do I run a safe first test? | [Production demo](production-demo.md) |
| How do I inspect duplicate candidates? | [Duplicate review](duplicate-review.md) |
| How do I handle privacy, retries, retention, and failures? | [Operations](operations.md) |
| How do smart folders fit in? | [Smart folders](smart-folders.md) |

## Sweep guides 🔎

- [OCR](sweeps/ocr.md) — text inside images.
- [Faces](sweeps/faces.md) — face presence/count, not identity.
- [Face identity](sweeps/face-identity.md) — human-reviewed identity workflow.
- [GPS/GIS](sweeps/gps.md) — coordinates and geographic enrichment.
- [Swatch / duplicates](sweeps/swatch.md) — visual similarity candidates.

## Developer guides

1. [Architecture](architecture.md) — stages, boundaries, and report contracts.
2. [Function reference](function-reference.md) — what each call does and why it exists.
3. [Development](development.md) — builds, checks, portability, contribution rules.
4. [System inventory](system-inventory.md) — external tools and workstation-derived pieces.
5. [Csharp integration](integration-csharp.md) — optional HEIC/HEIF conversion pipeline.
6. [Rules](../rules/photosweep.yaml) — thresholds and safety policy.

## A useful mental model

PhotoSort is not one magic “clean my photos” button. It is a chain of evidence-producing tools:

```text
source media
   ↓
verified staging copy
   ↓
metadata + hashes + raster inspection
   ↓
independent analysis sweeps
   ↓
reports and candidate groups
   ↓
human review
   ↓
explicit organization / cleanup
```

That separation is intentional. **Detection is not deletion. Similarity is not identity. A timestamp is evidence, not divine revelation.** 📸

## Historical code

The old PHP implementation and historical research material no longer live on the active `master` tree. They are preserved on the `archive/legacy-snapshot` branch so the working tree stays focused while the project's history remains available.