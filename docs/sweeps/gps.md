# GPS/GIS sweep

Command:

```sh
bin/photosweep run gps PHOTO_ROOT REPORT_DIR
```

The runner requests `GPSLatitude`, `GPSLongitude`, `GPSAltitude`, and `GPSDateTime` from ExifTool. It records a local `found` or `none` result. Coordinate-to-place geocoding is disabled by default to prevent accidental location disclosure and network dependence.

Treat `gps.jsonl` as sensitive. Do not commit it to GitHub, attach it to an issue, or publish it in a demo. If a future GIS stage is added, it should consume a copied report and make network use explicit.
