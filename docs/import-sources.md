# Getting source photos into PhotoSort 📥

PhotoSort analyzes local files. The safest import pattern is boring on purpose:

```text
device / cloud / archive
        ↓
untouched source copy
        ↓ verify counts + sizes + hashes when practical
working staging copy
        ↓
PhotoSort / Csharp / metadata repair
```

**Do not make your only copy the experimental copy.** Photo libraries are unusually good at containing 15 years of irreplaceable data plus three copies of a meme from 2014.

## Before every import

Create a source-specific staging directory and record where it came from:

```sh
mkdir -p ~/Photos/imports/2026-08-27-camera-card
```

Prefer copy operations over move operations. After copying, compare file counts and total bytes. For important removable media, consider a SHA-256 manifest:

```sh
find /path/to/import -type f -print0 | sort -z | xargs -0 sha256sum > import.sha256
```

Do not erase the source device/card until the imported copy opens correctly and your backup exists.

## Camera SD card / USB card reader 📷

This is the cleanest pipeline for most dedicated cameras.

1. Lock the SD card if it has a physical write-protect tab.
2. Mount it read-only when practical.
3. Copy the entire `DCIM` tree, not merely the JPEGs you recognize.
4. Preserve RAW+JPEG pairs, videos, sidecars, and camera-created filenames.
5. Verify the copy before formatting the card in-camera.

Example on Linux:

```sh
rsync -a --info=progress2 /media/$USER/CAMERA/DCIM/ ~/Photos/imports/camera/DCIM/
```

### Example: RAW + JPEG shooter

A camera may produce `DSC_0421.NEF` and `DSC_0421.JPG`. They are two representations of one shutter event, **not byte duplicates**. Keep both during ingestion. PhotoSort's identity hash correctly treats them as separate files; later policy can decide whether both remain in the working library.

## Android phone 🤖

### USB / MTP

Unlock the phone, select **File transfer**, and copy `DCIM/Camera` plus any other desired media directories. MTP is not a normal POSIX filesystem, so copying through a desktop file manager or an MTP-aware tool may be more reliable than assuming `rsync` semantics.

Common places worth checking include:

```text
DCIM/Camera
Pictures/
Download/
Movies/
```

Messaging apps often have their own directories. Treat those as separate sources because their media may have stripped or rewritten metadata.

### ADB pipeline

For a phone with USB debugging already configured:

```sh
adb devices
adb pull /sdcard/DCIM/Camera ~/Photos/imports/android-camera
```

Verify the local result before deleting anything from the phone.

## iPhone / iPad 🍎

Three useful routes exist:

- **USB import:** use the OS photo importer or a libimobiledevice-compatible workflow and preserve original files when offered.
- **iCloud Photos export:** useful for the current cloud library.
- **Apple Data & Privacy export:** useful for a larger archival export.

Do not silently convert HEIC to JPEG during the only archival import. Keep HEIC originals; generate JPEG derivatives later with Csharp if needed.

## Apple iCloud / Apple Data & Privacy

1. Request the photo export from Apple's privacy/data tools.
2. Download **every** archive part.
3. Keep the original archives untouched.
4. Extract into a staging tree.
5. Preserve sidecars, albums, edits, and metadata files.
6. Use Csharp for HEIC/HEIF derivatives only when needed.

Cloud exports can contain multiple representations of the same logical photo. Filename equality is not proof; SHA-256 answers the narrower question “are these exact same bytes?”

## Google Takeout 📦

1. Export Google Photos.
2. Download every archive part before cleanup.
3. Extract all parts into one source-specific staging area.
4. **Keep the JSON sidecars.** They can contain timestamps, descriptions, and coordinates missing from the media file.
5. Inventory before attempting metadata repair.

### The classic Takeout case

You have:

```text
IMG_1234.jpg
IMG_1234.jpg.json
```

The JPEG says it was modified yesterday because an export process touched it, while the JSON says it was captured in 2017. Do **not** bulk-copy filesystem mtime into EXIF. The sidecar may be stronger evidence.

## OneDrive with rclone ☁️

Configure the remote with `rclone config`, then start with `copy`, not `sync`:

```sh
rclone copy --progress --checksum --transfers 4 onedrive:Pictures ~/Photos/imports/onedrive/Pictures
rclone check onedrive:Pictures ~/Photos/imports/onedrive/Pictures --one-way
```

`sync` is useful when you truly want mirroring. During rescue/import work, its ability to delete destination files is an exciting feature in exactly the wrong way.

## Facebook / Meta / social exports 🌐

Request the highest-quality available export and preserve its HTML/JSON metadata. Social platforms frequently recompress images, strip camera metadata, or replace timestamps. Treat social copies as **secondary evidence**, especially when an original camera/phone copy exists elsewhere.

The same principle applies to Instagram, messaging apps, old forums, and downloaded attachments.

## Old hard drives and mystery folders 💽

For an old disk:

1. Copy first; organize later.
2. Preserve the original directory tree inside a source-named folder.
3. Do not merge similarly named folders by hand yet.
4. Inventory exact hashes.
5. Run metadata and visual analysis only after the rescue copy is stable.

If the disk is physically failing, PhotoSort is not a data-recovery tool. Image/clone the disk with an appropriate recovery workflow first.

## Fixing damaged or missing EXIF 🧰

Read [What is EXIF?](exif.md) before bulk repair. Metadata repair should be **evidence based**.

### First: inspect, don't write

ExifTool is extremely useful for diagnosis:

```sh
exiftool -G1 -a -s IMG_1234.jpg
```

Look for `DateTimeOriginal`, `CreateDate`, GPS fields, camera make/model, orientation, and XMP/IPTC dates.

### Evidence priority for dates

A practical default order is:

1. intact `DateTimeOriginal` from the original camera file;
2. trustworthy sidecar/export metadata;
3. another original representation of the same shutter event;
4. filename patterns generated by a known device/app;
5. known folder/event context;
6. filesystem timestamps — **last resort**.

Filesystem mtime is easy to destroy by copying, extracting, downloading, or editing.

### Safe repair pattern

Never begin with a recursive overwrite. Prove the transformation on copies of 5–20 representative files:

```sh
mkdir /tmp/exif-test
cp IMG_1234.jpg /tmp/exif-test/
exiftool -overwrite_original -DateTimeOriginal='2017:06:14 18:42:10' /tmp/exif-test/IMG_1234.jpg
exiftool -G1 -a -s /tmp/exif-test/IMG_1234.jpg
```

Once the rule is proven, automate it and retain a repair log containing source path, old value, new value, evidence source, and tool version.

### What *not* to invent

If GPS is absent, “probably Boston” is not GPS. If the exact capture time is unknown, do not manufacture seconds of precision. Store uncertainty in your report or sidecar instead of turning a guess into apparently authoritative EXIF.

## After import: the handoff

A healthy import ends with:

```text
✅ original/source preserved
✅ staging copy readable
✅ counts checked
✅ sidecars retained
✅ source recorded
✅ no destructive cleanup yet
        ↓
run PhotoSort inventory / sweeps
```

Then see [Workflow](workflow.md) and [What is EXIF?](exif.md).