# Face-detection integration report

Date: 2026-08-11

Status: v1 implemented and bounded-tested. No full-library backfill was run.

## Current architecture

```text
Documents/ Downloads/ Pictures/ Desktop/
        |
        | Recoll topdirs + monitordirs
        v
recollindex -n -m -D -x -w 20
        |
        +-- Recoll MIME dispatch
        |     +-- JPEG/PNG/TIFF/BMP/WEBP -> ~/.local/lib/ocr-search/rclocr-image
        |     |                              -> Pillow size gate
        |     |                              -> /usr/share/recoll/filters/rclocr.py
        |     |                              -> rclocrtesseract.py -> tesseract
        |     |                              -> OCRCache
        |     +-- image-only PDF -> Recoll rclpdf.py/rclocr.py path, 300 dpi pages
        |     +-- GIF/JP2/NEF/XCF -> Recoll rclimg or other built-in handlers
        |
        v
Xapian index: ~/.cache/recoll-ocr-search/xapiandb
OCR cache:    ~/.cache/recoll-ocr-search/ocrcache
        |
        v
recoll GUI / ocr-search -> recollq -> original file paths + snippets
```

The active index is Recoll 1.36.1 with Xapian 1.4.22. There is no SQLite
database, generic extractor framework, face index, or active photo metadata
daemon.

## Important components

| Path | Purpose | Language | State / relationships |
|---|---|---|---|
| `~/.config/systemd/user/recoll-ocr-monitor.service` | Runs continuous Recoll monitoring | systemd unit | Enabled and active; starts `/usr/bin/recollindex` |
| `~/ocr-search-setup/recoll.conf` | Install-time Recoll configuration template | config | Source for the active config |
| `~/.recoll/recoll.conf` | Live roots, cache, OCR, throttling, exclusions | config | Active |
| `~/ocr-search-setup/mimeconf` / `~/.recoll/mimeconf` | MIME-to-filter mappings | config | Active; image OCR overrides are present |
| `~/ocr-search-setup/mimemap` / `~/.recoll/mimemap` | Extension-to-MIME mappings | config | Active |
| `~/.local/lib/ocr-search/rclocr-image` | Rejects tiny images, delegates OCR | Python 3 + Pillow | Called by MIME rules |
| `/usr/share/recoll/filters/rclocr.py` | OCR cache lookup/store and backend dispatch | Python 3 | Called by the wrapper |
| `/usr/share/recoll/filters/rclocrtesseract.py` | Tesseract execution and PDF rasterization | Python 3 | Called by `rclocr.py` |
| `/usr/bin/tesseract` | OCR engine | native C++ | Active; configured language `eng`, one thread |
| `~/.local/bin/ocr-search` | Simple search launcher | Bash | Calls `recollq` |
| `~/.local/bin/ocr-reindex` | Bounded file/directory reindex | Bash | Stops monitor, runs low-priority Recoll, restarts it |
| `~/.local/bin/ocr-index-status` | Status/cache/log report | Bash | Read-only helper |
| `~/.local/bin/ocr-search-pause` / `ocr-search-resume` | Pause/resume monitor | Bash | Active helpers |
| `~/photo-sweep.sh` | One-shot EXIF classification, move/dedupe/report tool | Bash + ExifTool/jq/Perl | Not called by OCR service; no unit found |
| `~/lib/photo-similarity.pl` | Similar-image helper for photo-sweep | Perl | Called only by photo-sweep |

`~/ocr-search-setup` is the identifiable project directory, but it is not a
Git repository. The report is saved there; no production file was changed.

## Services and triggers

The only relevant active unit found is:

```text
recoll-ocr-monitor.service (enabled, active)
  ExecStart=/usr/bin/recollindex -m -D -x -w 20 -c %h/.recoll
  Nice=15, IOSchedulingClass=idle, CPUWeight=20, IOWeight=20
```

Recoll's `-m` mode is the watcher/scanner. `monitordirs` matches `topdirs`:
`Documents`, `Downloads`, `Pictures`, and `Desktop`. No separate inotify
script, path unit, timer, OCR daemon, or `photo-sweep.service` was found.

There is an important live drift: the unit file and `systemctl show` report the
command without `-n`, but the running PID 3852 command line is:

```text
/usr/bin/recollindex -n -m -D -x -w 20 -c /home/freezer/.recoll
```

Recoll documents `-n` as disabling the initial incremental indexing and purge.
This should be reconciled deliberately before any future backfill; do not
blindly restart it during this inspection.

## Formats and extraction

The active custom OCR rules cover JPEG, PNG, TIFF, BMP, and WEBP. The wrapper
skips images smaller than 160x80 or 40,000 pixels, then invokes Recoll's
`rclocr.py`. The Tesseract backend directly runs:

```text
tesseract IMAGE stdout -l eng
```

For image-only PDFs, the built-in OCR path rasterizes pages at 300 dpi with
`pdftocairo` or `pdftoppm`, then runs Tesseract serially per page. GIF, JP2,
NEF, and XCF use other Recoll handlers; they are not in the custom OCR image
override. The current index contains 33 JPEG, 34 PNG, 1 BMP, 13 GIF, and 174
PDF documents; TIFF/WEBP/NEF had zero current results (counts are not a
library inventory).

Recoll's normal `rclimg` handler uses Perl Image::ExifTool and maps selected
metadata such as title, caption, author, keywords, and tags to Recoll fields.
However, JPEG/PNG/TIFF/BMP/WEBP are overridden by `rclocr-image`, which emits
plain OCR text rather than the normal `rclimg` HTML metadata. A sampled JPEG
document dump contained path, size, MIME, mtime, and OCR-related fields but no
EXIF camera/model fields. ExifTool metadata is therefore not currently a
general searchable layer for those formats.

`photo-sweep.sh` does extract extensive ExifTool JSON, but writes a TSV
manifest/reports and optionally moves or deletes source files. It is separate
from Recoll and was not active.

## Storage and processed-file behavior

Recoll stores the searchable document/index data in Xapian glass files under:

```text
~/.cache/recoll-ocr-search/xapiandb/    about 259 MiB
~/.cache/recoll-ocr-search/ocrcache/    under the same cache root
```

No sidecars or xattrs are created by the OCR setup. OCR cache data is zlib
compressed and split into:

```text
ocrcache/paths/   path hash -> data hash, mtime, size, encoded path
ocrcache/objects/ data hash -> OCR result
```

The fast cache hit uses path + mtime + size. If that misses, Recoll hashes the
full file contents and can reuse OCR after a move; it then updates the path
record. This is safe for unchanged content, but an mtime/size collision is a
known limitation. Deleted cache path entries are not automatically purged by
the custom helpers.

Recoll handles changed files through monitor reindexing and supports explicit
purge (`-e`), individual indexing (`-i`), recursive partial indexing (`-r`),
retry (`-k`), and reset/force modes (`-z`, `-Z`). `ocr-reindex` only accepts
paths below the four configured roots, pauses the monitor, uses `nice` and
`ionice`, and restores the monitor with a shell trap. A reboot resumes the
monitor, but there is no separate durable face work queue or pause state.

## Search interface

The launcher is:

```bash
ocr-search "words to find"
# equivalent core operation:
recollq -c "$HOME/.recoll" --paths-only -A -n 100 "words to find"
```

The GUI launcher runs `recoll`; the desktop shortcut is `Ctrl+Super+F`.
Recoll query fields already support examples such as `mime:image/jpeg` and
`camera` is not currently a known field in this index. Search returns original
paths; OCR text is stored in the Xapian document for abstracts/snippets.

## Performance findings

Processing is effectively serial: the live Recoll process has two tasks, while
`tesseractnthreads = 1` limits Tesseract's OpenMP threads. It is intentionally
background-prioritized (`Nice=15`, idle I/O, CPU/IO weight 20). The service has
accumulated substantial CPU time and is still active; no additional OCR worker
pool or work queue exists.

The OCR wrapper uses Pillow only for a size check. It does not create or reuse
thumbnails, and the Tesseract path decodes the original image. PDF OCR creates
temporary raster pages. The Recoll cache prevents repeat OCR but does not reuse
a decoded image between OCR and a future detector.

Removable media is not specially modeled: only currently existing configured
directories are indexed, and no removable-drive service or availability queue
was found. Recoll can purge missing files according to its normal behavior,
but the live `-n` drift specifically suppresses the initial purge/incremental
pass.

## Recommended insertion point

There is no generic extractor interface. The least invasive extension point is
the existing image MIME filter boundary:

```text
image MIME dispatch
  -> combined OCR + face filter (future sibling/ replacement for rclocr-image)
       -> existing Recoll OCRCache/rclocr.py/Tesseract path
       -> face detector on a bounded downscaled image
       -> HTML meta fields returned to Recoll
  -> Xapian document + configured custom fields
```

Do not put face data in `photo-sweep` reports: those are outside the active
search index. Do not create a second database for the initial booleans/count.

The future filter should preserve the current OCR cache behavior and add a
small detector-result cache keyed by file content hash plus detector version
and parameters. It should emit canonical field names such as:

```html
<meta name="facespresent" content="true">
<meta name="facescount" content="3">
<meta name="facesdetector" content="opencv-haar">
```

Recoll custom field names must be lowercase alphabetic ASCII, so dotted names
like `faces.count` should not be used directly. Configure `~/.recoll/fields`
with a searchable prefix for `facespresent` and `facesdetector`, and a numeric
Xapian value slot for `facescount` (value slots above 1000 are available).
Then the supported range form is `facescount:3..` rather than SQL-style `>=`:

```text
facespresent:true
facescount:1..
facescount:3..
facescount:3.. camera:Canon
facespresent:true facescount:3.. ocr:"birthday"
```

An alias can make `faces:true` map to `facespresent:true`; a query alias or
wrapper can make `faces:>=3` map to `facescount:3..` if that exact syntax is a
hard requirement. Test this against the installed Recoll version before
exposing it in `ocr-search`.

## Detector recommendation

Primary recommendation: OpenCV C++ `CascadeClassifier` with a vetted frontal
face Haar cascade, initially run on a downscaled grayscale image. OpenCV's
native API directly supports Haar/LBP XML cascades and returns rectangles via
`detectMultiScale`; Ubuntu's package metadata shows `libopencv-objdetect-dev`
and `opencv-data` are available but not installed. It fits the existing
headless, low-priority native-tool preference and avoids Python bindings.

Use Haar first because the immediate requirement is a cheap, explainable
`present/count` signal. Validate thresholds and false positives on a fixed
20-photo corpus including portraits, groups, posters, statues, pets, and
text-heavy images. LBP is a fallback within OpenCV when throughput matters
more than recall; it is generally lighter but often less tolerant of pose and
lighting variation.

Fallback: dlib's native HOG frontal detector. It has a compact C++ API and can
handle some pose variation better, but adds a larger C++ integration/build
surface and is still not a general profile/occlusion detector. Do not use
dlib's CNN detector or any embedding/identity functionality for this phase.

The recommendation is based on the local architecture and package availability
plus the official APIs: OpenCV documents Haar/LBP cascades and
`detectMultiScale`; dlib documents its FHOG/HOG frontal detector. Neither
requires Python when integrated as a small native helper.

## Minimal future implementation plan

1. Reconcile the service command-line drift and decide whether initial
   incremental indexing/purge should be enabled; record the decision.
2. Install only the selected OpenCV runtime/development/data packages after a
   package-size review. Do not install dlib unless the Haar evaluation fails.
3. Add a small C++ headless helper, or a minimal native filter executable,
   accepting one image and returning `present`, `count`, and optionally boxes.
   Load/resize once and cap the detector input dimensions.
4. Add a combined Recoll image filter that reuses the existing OCR cache and
   adds detector-result caching keyed by content hash + detector version +
   parameters. Keep detector failure non-fatal to OCR.
5. Add `~/.recoll/fields` source/config entries for `facespresent`,
   `facescount`, and `facesdetector`; use a numeric value slot for count.
6. Reindex only a small representative sample and verify `facespresent:true`,
   `facescount:1..`, `facescount:3..`, and combinations with MIME/OCR fields.
7. Enable detection for newly changed image files through the existing Recoll
   monitor. Do not alter Thunar or photo locations.
8. Add a separate throttled backfill command that selects indexed image paths
   not yet covered by the current detector-version cache. Process one item at a
   time with `nice`/idle I/O, skip unavailable roots, persist progress in the
   detector cache, and stop cleanly on SIGTERM.
9. Provide explicit pause/resume/status commands and a dry-run/count mode.
   Run the legacy backfill only after detector quality and query behavior are
   accepted.

## Problems and limitations found

* The active custom OCR path is Python/Pillow-based; this is existing project
  behavior, not a proposed new dependency.
* The custom image override likely sacrifices normal searchable EXIF fields
  for the covered image MIME types; the sampled JPEG confirms no camera fields
  in its Recoll document record.
* `photo-sweep.sh` is operationally separate and includes destructive moves and
  duplicate deletion. It must not be used as the face-index orchestrator.
* The running Recoll command has undocumented `-n` relative to the unit file.
* There is no durable extractor queue, detector version column, or thumbnail
  reuse layer today.
* The active log showed repeated updates for non-photo files in Downloads,
  consistent with the configured roots being broad rather than photo-only.

## Safe inspection commands

```bash
systemctl --user cat recoll-ocr-monitor.service
systemctl --user status recoll-ocr-monitor.service --no-pager -l
ocr-index-status
recollq -c "$HOME/.recoll" -m -n 1 'mime:image/jpeg'
recollindex -c "$HOME/.recoll" --notindexed "$HOME/Pictures/some-file.jpg"
du -sh "$HOME/.cache/recoll-ocr-search"/*
journalctl --user -u recoll-ocr-monitor.service --since today --no-pager
```

No questions are required to understand the current machinery. Before
implementation, the remaining product choices are detector thresholds,
whether to retain/restore EXIF fields in the combined handler, and the exact
user-facing shorthand for numeric face queries.


## Implementation (2026-08-11)

### Implemented

- Added native C++ detector source: `face-detect.cpp`.
- Added build script: `build-face-detect.sh`.
- Installed binary: `~/.local/lib/ocr-search/face-detect`.
- Added combined filter: `rclocr-image-combined`, installed under `~/.local/lib/ocr-search/`.
- Added versioned face cache under `~/.cache/recoll-ocr-search/facecache/`.
- Added Recoll fields in `~/.recoll/fields` and source `ocr-search-setup/fields`.
- Added deferred helper `~/.local/bin/face-backfill`; it has not been run.
- Updated active and source MIME mappings to return HTML from the combined filter.

### Updated pipeline

```text
image
  +-- ExifTool metadata -> HTML meta/body fields
  +-- existing rclocr.py -> existing OCRCache -> Tesseract OCR
  +-- face-detect -> OpenCV Haar -> versioned face cache
          \-> facespresent/facescount/facesdetector HTML meta fields
                 \-> Recoll/Xapian
```

### Detector

- OpenCV 4.6.0 Ubuntu packages: `libopencv-objdetect-dev` and `opencv-data`.
- Cascade: `/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml`.
- Preprocessing: EXIF orientation, BGR decode, max dimension 1400, grayscale, histogram equalization.
- Parameters: `scaleFactor=1.10`, `minNeighbors=5`, `minSize=32x32`.
- Detector identifier: `opencv-haar-v1-opencv-4.6.0`.
- Native output supports key/value output and `--json` boxes.

### EXIF preservation

The former plain-text image override is now a combined HTML filter. It calls
ExifTool for normal image tags, emits searchable `title`, `author`, `keywords`,
`date`, `camera`, `cameramake`, `cameramodel`, `imagewidth`, and `imageheight`
fields, and includes the complete tag set in the indexed body. It then retains
the existing Recoll OCR filter/cache and appends face fields. EXIF-bearing
validation on `P7220249.JPG` produced `OLYMPUS IMAGING CORP.`,
`StylusTough-8010`, date `2014-07-22 05:07:02`, and dimensions `4288x3216`.
That mounted file was not indexed or modified.

### Recoll fields and queries

Configured searchable/stored fields are `facespresent`, `facescount`,
`facesdetector`, `camera`, `cameramake`, `cameramodel`, `imagewidth`, and
`imageheight`. `facescount` uses Xapian value slot 1001 as an integer. Verified:

```text
faces:true                         -> 3 results
facespresent:true                 -> 3 results
facescount:1                      -> 3 results
facescount:1..                    -> 3 results
facescount:3..                    -> 0 results
facespresent:true filename:...    -> 1 result
```

Range clauses must be written before the other clause or joined with `AND`;
`facescount:1.. filename:...` is valid, while the reverse order without `AND`
can be parsed ambiguously by this Recoll version.

### Bounded tests

The native detector was run on 20 existing images: 3 positive detections, 17
negative detections, no crashes. Positive files were
`1724790612063565.jpg`, `1737296217467572.jpg`, and
`1773375618067676.jpg`, each detected as one face. The remaining selected
wallpapers, UI images, and two `photo_*.jpg` files returned zero. This is a
technical smoke set, not a labeled accuracy benchmark.

The same 20 paths were individually reindexed with `recollindex -i -Z` while
the monitor was fully stopped. OCR was verified with
`protection filename:whisker-menu-drives.png` (1 result). Face field queries,
exact counts, range counts, and combinations were verified. The service is
now enabled, active, and systemd-owned.

Measured direct combined-filter timing on an EXIF-bearing 4288x3216 JPEG:
first run 7.51 seconds (cold OCR/face/metadata path), repeated run 0.50
seconds (cache hit). A 7680x1440 indexed wallpaper took 1.55 seconds on the
first direct run. These measurements include ExifTool and OCR; they are not a
full-library throughput estimate.

### Deferred backfill

Do not run automatically. The prepared command is:

```bash
~/.local/bin/face-backfill
```

It reads currently indexed JPEG/PNG/TIFF/BMP/WEBP paths from Recoll, pauses
the monitor, uses `recollindex -i -Z` one file at a time with `nice`/`ionice`,
relies on the versioned cache for resume behavior, skips missing/failed paths,
and restores the monitor. It is interruptible and removable-drive-safe.

### Known limitations

- Haar v1 misses profiles, occluded faces, very small/distant faces, and poor
  lighting; the 20-file set was not manually labeled.
- The initial face cache stores path records and compact result objects, not
  bounding boxes.
- Existing images outside the bounded sample have not been backfilled.
- Camera fields are available only for images whose source metadata contains
  those tags.
- The combined filter currently emits custom camera fields; the normal raw
  ExifTool tags remain in the indexed body.

### Rollback

To disable future face enrichment while preserving the existing Xapian/OCR
index and caches, restore the previous five image MIME lines in
`~/.recoll/mimeconf` to point to `~/.local/lib/ocr-search/rclocr-image` with
`mimetype = text/plain`, then remove only the combined filter, fields file,
face-backfill helper, and face cache if desired:

```bash
systemctl --user stop recoll-ocr-monitor.service
install -m 0644 ~/ocr-search-setup/backups/20260715-221442/mimeconf ~/.recoll/mimeconf
rm -f ~/.local/lib/ocr-search/rclocr-image-combined ~/.local/bin/face-backfill ~/.recoll/fields
rm -rf ~/.cache/recoll-ocr-search/facecache
systemctl --user enable --now recoll-ocr-monitor.service
```

This rollback does not delete `xapiandb` or `ocrcache`. A later reindex of
images is required if old plain-text document contents must be restored; no
full rollback reindex was run here.
