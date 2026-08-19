# Face sweep

Command:

```sh
bin/photosweep run faces PHOTO_ROOT REPORT_DIR
```

The runner calls the native `face-detect` executable when it is available. The expected adapter output is key/value text such as:

```text
facespresent=true
facescount=2
facesdetector=opencv-haar-v1
```

No identity recognition is performed. Results are presence/count signals only. No XMP or other metadata is written by the sweep.

If the detector is missing, each record is marked `unavailable`. This is deliberate: a demo can still complete OCR, GPS, and swatch analysis without pretending faces were checked.
