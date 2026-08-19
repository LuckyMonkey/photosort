# Operations, privacy, and recovery

## Before a run

- Work from a read-only mount or a verified copy for the first pass.
- Use a new report directory; reports are append-only.
- Confirm free space for text reports and temporary tool output.
- Keep GPS and OCR reports outside the Git worktree.
- Record the Git commit and installed tool versions.

## During a run

The four sweeps are independent. A missing optional dependency should produce `unavailable`, not stop unrelated passes. If a tool fails repeatedly, preserve the partial JSONL and rerun only that sweep into a new directory after fixing the dependency.

## After a run

Review record counts, status counts, and a sample of each report. For faces, inspect false positives on portraits, groups, posters, statues, pets, and text-heavy images. For GPS, treat every coordinate as sensitive.

## Recovery

Analysis creates no source-file mutation to undo. Delete or quarantine generated reports if they contain sensitive data. Never publish library reports, OCR text, GPS coordinates, or face results in a public issue.
