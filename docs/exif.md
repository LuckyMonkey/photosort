# What is EXIF? 📸 A field guide to what your camera remembers

EXIF (**Exchangeable Image File Format**) is metadata commonly embedded in image files by cameras and phones. The pixels answer **what the picture looks like**; metadata can answer questions such as **when was it taken, with what camera, at what exposure, in what orientation, and sometimes where?**

EXIF is not the only metadata system. Real photo libraries commonly contain EXIF, XMP, IPTC, container-specific metadata, maker notes, filesystem timestamps, and external sidecars. PhotoSort treats these as evidence rather than assuming one field is infallible.

## A photo is more than pixels

Imagine this file:

```text
IMG_4821.JPG
```

The visible photograph might be a dog jumping through snow. Hidden inside the file might be information resembling:

```text
Make:               Canon
Model:              EOS R6
Lens:               RF24-105mm F4 L IS USM
DateTimeOriginal:   2026:01:18 14:32:07
ExposureTime:       1/1600
FNumber:            4.0
ISO:                800
FocalLength:        105 mm
ExposureProgram:    Manual
WhiteBalance:       Auto
Flash:              Off
Orientation:        Horizontal
GPS:                [possibly present]
```

That little block can explain a surprising amount about how the image was made.

## The exposure triangle: aperture, shutter, ISO 🔺

A camera needs light. Three controls dominate ordinary exposure decisions.

### Aperture — how wide the lens opens

Aperture is written as an f-number such as **f/1.8, f/4, f/8, f/16**. A smaller f-number means a wider opening and generally more light. Aperture also influences depth of field.

- `f/1.8`: lots of light, potentially shallow depth of field.
- `f/4`: moderate opening.
- `f/8`: less light, often more depth of field.
- `f/16`: much smaller opening; diffraction may reduce peak sharpness on many systems.

In EXIF you will often see `FNumber` and sometimes `ApertureValue`.

**Use case:** a portrait at `85 mm, f/1.8` with the eyes sharp and background melted away makes sense. The metadata helps explain the rendering rather than merely naming the camera.

### Shutter speed — how long light is collected

Common values include:

```text
1/4000 s   freezes very fast action
1/1000 s   freezes many moving subjects
1/250 s    common handheld/action territory
1/60 s     camera shake becomes more relevant
1 s        obvious motion blur unless the scene is still
30 s       long exposure
```

EXIF usually records this as `ExposureTime` and may also contain `ShutterSpeedValue`.

**Use case:** two photos look almost identical, but one is sharp and one has smeared hands. EXIF reveals `1/500` versus `1/30`. Mystery solved.

### ISO — sensor amplification / exposure index

ISO values commonly look like `100`, `400`, `1600`, `6400`. Raising ISO lets the camera produce a brighter result for a given aperture and shutter combination, generally at the cost of more noise or reduced image quality/dynamic range depending on the camera.

EXIF usually exposes `ISO` or a related sensitivity field.

**Use case:** a dark concert photo says `1/250, f/2, ISO 6400`. The photographer chose enough shutter speed to stop motion, opened the lens, then accepted high ISO because the alternative was a blurry musician.

## Manual mode: the EXIF becomes a tiny shooting notebook 📝

Suppose you set a camera to **M** and choose:

```text
1/500 s
f/5.6
ISO 400
50 mm
```

The camera normally writes those actual capture settings into metadata. If you later change to `1/125, f/2.8, ISO 1600`, the next image records the new values.

This is useful for learning photography: shoot a controlled series, inspect the photos, then inspect their EXIF. You can see exactly what changed.

### Example: freezing a cyclist

You start with:

```text
Mode: M
Shutter: 1/250
Aperture: f/5.6
ISO: 200
```

The cyclist is slightly blurred. You change shutter to `1/1000`. That removes two stops of light, so you compensate by opening the aperture, increasing ISO, or both. Your next file might say:

```text
Shutter: 1/1000
Aperture: f/4
ISO: 400
```

Months later, EXIF preserves the technical story.

## Camera modes and what gets recorded

The exact tags vary by manufacturer, but EXIF may record an exposure program or mode.

### Manual (M)

You directly choose aperture and shutter; ISO may be manual or Auto ISO depending on the camera. The resulting values are recorded.

### Aperture priority (A / Av)

You choose aperture; the camera usually chooses shutter speed. EXIF records the resulting aperture and shutter values, not merely your intention.

### Shutter priority (S / Tv)

You choose shutter speed; the camera generally chooses aperture. Again, EXIF records the actual capture values.

### Program (P) / Auto

The camera chooses more of the exposure. EXIF can still record the final ISO, aperture, shutter speed, flash state, focal length, and other settings.

## Focal length and lens information 🔭

`FocalLength` tells you the lens's actual focal length at capture, such as `24 mm`, `50 mm`, or `200 mm`. Zoom lenses therefore create different values from shot to shot.

Some files also contain:

- lens make/model;
- minimum/maximum focal length;
- maximum aperture;
- 35 mm-equivalent focal length;
- lens serial information in maker-specific metadata.

Be careful comparing phone cameras with dedicated cameras. Phones may contain several physical cameras plus computational cropping, and “35 mm equivalent” is not the same thing as physical focal length.

## Exposure compensation

In semi-automatic modes, the photographer can ask the meter for a brighter or darker result, often shown as `+1 EV`, `-0.7 EV`, etc. EXIF may record this as `ExposureCompensation` or `ExposureBiasValue`.

This is valuable when asking, “Why did I deliberately make every snow photo brighter than the meter wanted?” The metadata may literally contain the answer.

## Metering, flash, white balance, and focus-related data

Depending on the camera, EXIF or manufacturer maker notes can include:

- metering mode;
- flash fired / did not fire;
- white balance;
- scene type;
- subject distance;
- focus mode;
- stabilization state;
- drive mode;
- picture style;
- digital zoom/crop;
- exposure bracketing;
- HDR-related settings.

Not every field is standardized equally. Maker notes are especially manufacturer-specific and can disappear when software rewrites a file.

## Date and time: one photograph can have several clocks ⏰

This is one of the most important parts of photo archiving.

Common fields include:

- `DateTimeOriginal` — generally the capture time;
- `CreateDate` / `DateTimeDigitized` — when the digital image data was created;
- `ModifyDate` — metadata/image modification time;
- sub-second fields;
- timezone/offset fields on newer devices;
- filesystem **mtime**, **ctime**, and sometimes birth/creation time outside EXIF.

These are not interchangeable.

### Example: why filesystem mtime lies

A photo was captured in 2012. In 2026 you download it from cloud storage. The filesystem says:

```text
mtime: 2026-08-27
```

but EXIF says:

```text
DateTimeOriginal: 2012-04-03 16:22:11
```

For photo chronology, the EXIF capture time is normally much stronger evidence. Copying and extracting files routinely changes filesystem timestamps.

### Time zones are evil little goblins

Older EXIF often records local wall-clock time without an explicit timezone. A file may say `2015:07:04 21:30:00` without saying whether that meant Boston, Denver, or Tokyo.

Modern phones may preserve offsets or additional location/timezone evidence, but archives should not assume every timestamp is globally unambiguous.

## GPS: EXIF can know where you were 📍

Phones and GPS-equipped cameras may embed:

- latitude;
- longitude;
- altitude;
- GPS timestamp/date;
- direction/bearing;
- positioning-related fields.

Coordinates can later be reverse-geocoded into place names, but **“Boston, Massachusetts” is derived information; latitude/longitude is the primary coordinate evidence.**

GPS is also sensitive personal data. A public photo can accidentally disclose a home address or repeated location pattern.

## Orientation: why a JPEG can look sideways in one program

Some cameras store pixels in one physical orientation and add an EXIF `Orientation` tag telling software how to display them. Good software honors the tag. Bad software shows the raw raster sideways. Other software rotates the pixels and resets orientation.

This matters during conversion: blindly copying an orientation tag after already rotating pixels can rotate the image twice.

## Camera identity

Common tags include:

```text
Make
Model
SerialNumber
LensModel
LensSerialNumber
Software
```

These are useful for organization and forensic provenance, but they are not guaranteed. Editors and social networks may remove them, and metadata can be edited.

## RAW versus JPEG versus HEIC/HEIF

Metadata handling depends on the format.

### JPEG

Commonly contains EXIF directly, plus optional XMP and IPTC metadata. It is widely supported but lossy.

### RAW

Camera RAW formats often contain extensive camera-specific metadata and an embedded preview. The RAW file should generally be treated as an original capture asset rather than casually rewritten.

### HEIC / HEIF

HEIF is a container format and may store metadata in container items rather than exactly like a JPEG. Conversion software must deliberately preserve or translate supported metadata. This is one reason PhotoSort keeps conversion separate through Csharp rather than pretending a format conversion is merely “change the extension.”

## EXIF versus XMP versus IPTC

A practical simplification:

- **EXIF:** camera/capture-oriented technical metadata.
- **IPTC:** descriptive/editorial fields such as captions, keywords, creator information.
- **XMP:** extensible metadata often used by editing/catalog applications; can live embedded or in sidecar files.

A professional workflow may have the original capture date in EXIF, star rating and edits in XMP, and caption/copyright information in IPTC-style fields.

## Sidecars: metadata living next door

You may encounter:

```text
IMG_1001.CR3
IMG_1001.xmp
```

or cloud-export JSON beside a JPEG. Sidecars are not inherently bad. They are useful when the original should remain immutable or the format cannot safely carry the application's metadata.

The problem is **separation risk**: rename or move the image without its sidecar and context disappears.

PhotoSort imports should therefore preserve sidecars until their information has been reconciled.

## What happens when you edit or upload a photo?

Anything can happen. 😅

Software may:

- preserve all metadata;
- preserve only selected EXIF;
- strip GPS for privacy;
- rewrite orientation;
- update software/modify tags;
- remove maker notes;
- export a new file with only a subset of metadata;
- put edits into XMP instead of changing the source;
- recompress pixels and generate a new file identity.

This is why a social-media download is not equivalent to the camera original even when it looks visually identical.

## Inspecting EXIF with ExifTool 🔬

A compact view:

```sh
exiftool photo.jpg
```

A better forensic view:

```sh
exiftool -G1 -a -s photo.jpg
```

Useful targeted fields:

```sh
exiftool -DateTimeOriginal -CreateDate -ModifyDate -Make -Model -LensModel \
  -ExposureTime -FNumber -ISO -FocalLength -GPSLatitude -GPSLongitude photo.jpg
```

For a directory, inspect before writing:

```sh
exiftool -r -csv -DateTimeOriginal -CreateDate -Make -Model -GPSLatitude -GPSLongitude /path/to/photos > metadata-audit.csv
```

## Corrupted EXIF: what does “corrupted” actually mean? 🧯

It can mean several different things:

1. metadata is structurally malformed;
2. a tag is missing;
3. a tag exists but contains a wrong value;
4. the image pixels are valid but metadata cannot be parsed;
5. an editor stripped fields;
6. cloud/social export moved metadata into sidecars;
7. timezone information was lost;
8. the camera clock itself was wrong.

Those require different fixes. “Copy mtime to every EXIF date” is not a universal repair; it is how a recoverable ambiguity becomes confidently wrong data.

## Evidence-based repair

Use the strongest available evidence and preserve provenance.

### Case 1: EXIF date missing, Google sidecar has capture time

Reasonable approach: validate the sidecar relationship, convert its timestamp correctly, write the missing capture field to a **working copy**, and log the change.

### Case 2: camera clock was exactly 7 hours wrong for an entire trip

Do not manually edit thousands of files. Prove the offset using known events, define the affected time/device boundary, test several samples, then apply a deterministic offset with a log and backup.

### Case 3: social-media copy has no camera model

If you have an exact/near-identical original elsewhere with reliable provenance, you may use that relationship to reconstruct catalog metadata. Do not blindly inject camera tags into the recompressed social copy and then forget they were reconstructed.

### Case 4: no date evidence exists

Leave it unknown or store a coarse estimated date outside authoritative EXIF. **Unknown is a valid answer.**

## Metadata is evidence, not truth

EXIF can be edited. Camera clocks can be wrong. GPS can be inaccurate. Firmware can write strange values. Exporters can mangle dates. A file claiming `Canon EOS R5` was not cryptographically signed by Canon.

PhotoSort therefore benefits from combining signals:

```text
exact hash
+ metadata
+ filename
+ sidecars
+ dimensions
+ source provenance
+ visual similarity
+ human review
```

No single field has to carry the entire archive on its back.

## What PhotoSort should preserve

For archival and organization work, useful principles are:

- never discard the original solely because a derivative looks fine;
- preserve metadata before normalization;
- distinguish observed values from reconstructed values;
- retain source provenance;
- log metadata mutations;
- prefer deterministic, reversible operations;
- treat GPS and face-related reports as private;
- use SHA-256 for exact byte identity, not filenames;
- treat visual similarity as a review hint, not proof.

## A final example: reading the photograph's technical story

Suppose EXIF says:

```text
Camera: Sony α7 IV
Lens: 35mm F1.4
Mode: Manual
Shutter: 1/320 s
Aperture: f/1.4
ISO: 3200
Exposure compensation: 0
Flash: Off
DateTimeOriginal: 2025:11:08 22:14:31
```

Without seeing the photo, you can make cautious technical inferences: it was probably a low-light situation; the lens was wide open; the photographer maintained a moderately fast shutter speed and accepted a higher ISO. What you **cannot** infer from EXIF alone is who is pictured, whether the camera clock was correct, or whether the metadata was later edited.

That distinction is the whole philosophy: metadata is incredibly useful when you know what question each field can actually answer.

---

Next: [Import pipelines](import-sources.md) for getting originals onto disk, or [Workflow](workflow.md) for turning those files into PhotoSort reports.