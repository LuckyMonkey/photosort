# Duplicate chooser

Start the dependency-free C review server with a swatch report:

```sh
bin/photosweep chooser reports/swatch.jsonl 8765
```

Then open `http://127.0.0.1:8765/`. The server binds to loopback only. It renders the report as escaped text and accepts JSON decision posts at `/decision`; decisions are appended to `duplicate-decisions.jsonl` in the current directory.

The chooser does not resolve a decision into a file operation. An operator or a future audited cleanup tool must interpret decisions against the original report and verify hashes before any mutation.

For a production review, retain:

- the input report;
- the decision log;
- the tool version and Git commit;
- a before/after file count;
- a backup or recoverable trash path if cleanup is later approved.
