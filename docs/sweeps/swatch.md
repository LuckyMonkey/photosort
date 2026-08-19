# Swatch and duplicate sweep

Command:

```sh
bin/photosweep run swatch PHOTO_ROOT REPORT_DIR
```

The sweep calculates a compact 1x1 ImageMagick color fingerprint and always records the SHA-256 identity. SHA-256 is suitable for exact-byte duplicate detection. A swatch is only a similarity clue: different photographs can share a color, and resized/re-encoded copies can have different hashes.

The safe policy is therefore:

```text
exact SHA-256 match  -> candidate exact duplicate
similar swatch       -> human review
chooser decision     -> separate JSONL record
delete/move          -> never automatic
```

The dHash/similarity work from the original workstation run is preserved in the archived statistics and workflow notes; it should remain a review signal, not a destructive rule.
