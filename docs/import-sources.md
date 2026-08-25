# Getting source photos into PhotoSort 📥

PhotoSort analyzes a local directory. First export or synchronize your source library, keep the original export untouched, and point the organizer/analyzer at a working copy or read-only mount.

## Apple iCloud / Apple Data & Privacy

1. Sign in at [privacy.apple.com](https://privacy.apple.com/).
2. Choose **Request a copy of your data** or the iCloud Photos data option.
3. Select the photo data, choose a manageable split size, and submit the request.
4. Download every archive Apple provides; keep the archive names and download manifest.
5. Extract into a staging directory, verify the archive count and checksums where available, then run Csharp for HEIC/HEIF derivatives if needed.
6. Keep the original export unchanged and use the extracted staging tree as PhotoSort input.

Apple exports can include sidecars, albums, edits, and metadata. Do not assume a filename or folder is the canonical photo; use SHA-256 identity and reports.

## OneDrive with rclone ☁️

Install [rclone](https://rclone.org/) from its official instructions, then configure an authenticated remote with `rclone config`. A typical read-only-oriented copy is:

```sh
rclone copy --progress --checksum --transfers 4 onedrive:Pictures /path/to/staging/onedrive/Pictures
rclone check onedrive:Pictures /path/to/staging/onedrive/Pictures --one-way
```

Use `copy`, not `sync`, until the local copy has been verified: `sync` can delete files at the destination. Preserve the rclone log, compare counts/sizes, and only then run PhotoSort.

## Google Takeout 📦

1. Open [takeout.google.com](https://takeout.google.com/).
2. Select **Google Photos**, choose the requested albums/data, and create the export.
3. Download all Takeout archives and extract them into a staging directory.
4. Keep the JSON sidecars with their media; they may contain dates, descriptions, and locations not embedded in the image.
5. Check for duplicate archive content before analysis, then run PhotoSort over the extracted media tree.

Takeout may contain multiple representations of one photo. Treat the report and SHA-256 identity as the evidence, not the archive folder name.

## Facebook / Meta export 🌐

1. In Facebook, open **Settings & privacy → Settings → Accounts Center → Your information and permissions → Download your information** (the direct [Download Your Information](https://www.facebook.com/dyi) page may also work).
2. Select the account, choose photos/videos and the required date range, and request the export in a high-quality format.
3. Download every archive and extract them into a separate staging directory.
4. Preserve the HTML/JSON metadata and album structure; Facebook media names and timestamps may be incomplete.
5. Run PhotoSort on the extracted media, then compare its identity and visual reports against the other source imports.

## Privacy and verification 🔒

Exports can contain GPS, faces, private messages, album names, and account metadata. Keep staging trees and generated reports private. Verify archive completeness before deleting downloads, and do not publish `files.tsv`, OCR, GPS, face, or visual reports.
