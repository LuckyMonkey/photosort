# Duplicate review and comparison tool 🖥️

PhotoSweep produces similarity candidates; a human makes the duplicate decision. Similarity is never an automatic delete rule.

## Report chooser

For the dependency-free native chooser:

```sh
bin/photosweep chooser reports/swatch.jsonl 8765
```

Open `http://127.0.0.1:8765/`. It binds to loopback only and records decisions without mutating source files. Retain the input report, decision log, tool version/Git commit, file counts, and a recoverable backup/trash path.

## Side-by-side comparison tool

The companion `photo-compare` server loads the TSV visual-swatch report and presents two images side by side. Its normal launch is:

```sh
cd /path/to/photo-compare
./photo-compare --report "/path/to/Photos/reports/visual-swatch-matches-overnight.tsv" --port 8765
```

Open `http://127.0.0.1:8765/`. It binds to loopback only. Controls:

- `A` keeps left and moves right to the review quarantine.
- `D` keeps right and moves left to the review quarantine.
- `W` keeps both.
- `S` skips.
- `X` moves both to the review quarantine.
- `Z` undoes the most recent move when recoverable.

Decisions append to `swatch-review-decisions.tsv`; rejected files go to `.swatch-review-trash/` first. Do not empty that quarantine until the decision log and backups have been checked.

The two choosers serve different report formats, but share the same safety principle: detection narrows the queue, and an operator approves mutation.
