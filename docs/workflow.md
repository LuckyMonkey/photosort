# PhotoSweep workflow 🧭

PhotoSort is deliberately staged. The goal is to make a huge messy library **observable before it becomes mutable**.

```text
📥 IMPORT
   device / cloud / old disk
        ↓
🧊 PRESERVE
   untouched source + verified staging copy
        ↓
🧾 INVENTORY
   paths · sizes · hashes · metadata
        ↓
🔎 ANALYZE
   OCR · faces · GPS · swatch · raster
        ↓
🧠 REVIEW
   candidate groups + human decisions
        ↓
🗂️ ORGANIZE
   explicit moves / metadata repair / derivatives
        ↓
🧹 CLEANUP
   only after reports + backups are verified
```

## 1. Import without “organizing” yet

Use [Import pipelines](import-sources.md) to copy media from cameras, phones, cloud exports, and old drives. Keep sidecars. Keep RAW+JPEG pairs. Keep HEIC originals.

**Why?** If you rename, flatten, convert, deduplicate, and repair metadata simultaneously, you destroy evidence about where a file came from and make mistakes harder to unwind.

## 2. Understand the metadata

Read [What is EXIF?](exif.md) before bulk date/GPS repair. `DateTimeOriginal`, filesystem mtime, a Google sidecar timestamp, and a filename date are different pieces of evidence.

Example:

```text
IMG_2017.jpg
EXIF DateTimeOriginal: missing
filesystem mtime: 2026-08-27
Google sidecar capture time: 2017-06-14
```

The 2026 filesystem time probably describes the export/download, not the shutter event.

## 3. Inventory first

Inventory establishes file identity and metadata before specialized analysis. Exact SHA-256 equality means exact byte identity; it does **not** mean two differently encoded files cannot represent the same photograph.

This distinction matters for:

```text
IMG_0421.CR3
IMG_0421.JPG
facebook_12345.jpg
edited_IMG_0421.jpg
```

They may all descend from one shutter event while remaining four distinct files.

## 4. Run independent sweeps 🔎

Each sweep should be useful without requiring every other optional dependency:

- OCR finds text signals.
- Faces records face presence/count.
- GPS extracts location evidence.
- Swatch/dHash creates visual-similarity candidates.
- Raster analysis helps distinguish dimensions/content classes.

A missing optional tool is recorded as `unavailable`; a failure is recorded as `error`. The pipeline should not silently improvise a result.

## 5. Review candidates; do not worship the algorithm

A visual match is a reason to look, not a deletion warrant. Open the [duplicate comparison workflow](duplicate-review.md) and decide what each pair means.

### Example: obvious duplicate

```text
iCloud/IMG_1004.JPG
OneDrive/IMG_1004.JPG
SHA-256: identical
```

This is strong exact-duplicate evidence.

### Example: visually similar but both valuable

```text
DSC_5001.JPG — person looking at camera
DSC_5002.JPG — same burst, person laughing
```

A perceptual matcher may group them. A human can correctly keep both.

## 6. Convert only when you need a derivative

If a workflow needs JPEG versions of HEIC/HEIF, use the companion [Csharp integration](integration-csharp.md). Preserve the HEIC original. A derivative is useful interoperability material, not a time machine that makes the source unnecessary.

## 7. Repair metadata as a separate operation 🧰

Metadata repair should happen only after the source and evidence have been inventoried. Log old value, new value, evidence source, and tool version. Test a rule on a small representative batch before touching thousands of files.

## 8. Organize after evidence exists

The organizer can create date/camera/location-oriented structures after metadata decisions are understood. Retain reports with the organized library so future-you can answer, “Why the hell is this file in this folder?” without archaeology.

## 9. Cleanup is the final gate

Only delete/quarantine originals or duplicate copies after:

- source imports are verified;
- reports are retained;
- duplicate decisions are reviewed;
- backups exist;
- the operation is explicit and recoverable where possible.

## Face identity is intentionally separate

The face-detection pass records presence/count only. Identity uses chunked, human-reviewed comparisons with a `not-a-subject` decision for false positives. People and pets are both first-class identity subjects. See [Face identity](sweeps/face-identity.md).

## Privacy 🔒

Generated reports can be more sensitive than the pictures appear individually. GPS reveals travel/home patterns; OCR can expose documents and account information; face analysis can expose who appears repeatedly. Keep reports local and do not publish them with bug reports or repository commits.

## The short version

```text
COPY → VERIFY → INVENTORY → ANALYZE → REVIEW → MUTATE
```

If you are about to mutate before you can explain what evidence you have, go one stage backward.