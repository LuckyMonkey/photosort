# PhotoSweep workflow

The pipeline is deliberately staged:

1. Inventory files and read metadata without changing them.
2. Run one or more independent sweeps. Each produces JSONL with the source path, file identity, tool/version, and result.
3. Review swatch/dHash groups in the duplicate chooser.
4. Export decisions for a later, explicit cleanup or organization pass.

The planned face-identity pass is separate from `faces`: face detection records presence/count only. Identity uses chunked, human-reviewed pairwise comparisons with a third `not-a-subject` decision for visual false positives. People and pets are both first-class identity subjects. See [Face identity warm-up and active learning](sweeps/face-identity.md).

The four passes are independent so a failed OCR or face dependency does not prevent GPS or duplicate analysis. A missing optional tool is recorded as `unavailable`, not guessed around.

The legacy organizer remains useful for date/camera folder creation. Its reports should be retained with the organized library; they contain SHA-256, dHash, dimensions, camera make/model, chosen date, and whether the date came from EXIF or filesystem mtime.

## Privacy

GPS reports may reveal home and travel locations. Keep them local and do not publish generated reports. Face results are currently counts/presence signals, not identity labels. Any future identity labels require explicit human confirmation and local metadata writes.
