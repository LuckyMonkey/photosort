# Smart-folder views

The planned location view is:

```text
LOCATIONS/
  California/
  Massachusetts/
  New Hampshire/
  Unknown/
```

`tools/build-location-smart-folders.sh` consumes the private
`geo-normalize-exif.py` TSV and creates symlink views. Originals remain in
place; no metadata or photo files are rewritten. Duplicate basenames receive a
short SHA-256 suffix.

Usage:

```sh
tools/build-location-smart-folders.sh /private/geo.tsv /path/to/view [state-svg-dir]
```

State icons are optional SVG assets named by USPS code (`CA.svg`, `MA.svg`,
`NH.svg`). The intended artwork is a green-filled state boundary vector. No
local boundary dataset was found, so the repository does not fabricate state
shapes or download private-library data. `Unknown/` contains unresolved GPS
records.

This is a view layer, not a second catalog: Recoll/OCR remains the search
index, EXIF remains the source of location tags, and swatch data remains in its
existing comparison tool.
