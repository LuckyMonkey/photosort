# Subject identity warm-up and active-learning pass

This is a planned follow-up to the current face-presence sweep. The method is human-in-the-loop pairwise subject verification with active learning and hard-negative mining. It covers human identities and household pets equally; the system must not discard or deprioritize an animal merely because it is not human.

The current detector must not be treated as an identity system. It may miss pets and has shown false positives for wood knots, statues, posters, and other face-like patterns. Identity work begins with a broader subject detector/filter so human faces, pet faces, and full pet subjects can all enter review.

## Review interaction

The warm-up chooser presents two aligned subject crops—human face, pet face, or pet body—and asks:

- **same subject**
- **different subject**
- **not a subject**

For a new cluster, it asks for subject type (`person` or `pet`) and an optional confirmed identity label such as a person’s name or a pet’s name. A household may have multiple pets of the same species, so species is not identity. The reviewer may leave it **unknown**. Unknown is valid; the system must never guess a person’s or pet’s identity.

The third choice means **not a subject** and supplies hard negatives for knotholes, statues, illustrations, avatars, text artifacts, and other non-living visual patterns. Animals are not hard negatives: pets are first-class subjects and must be clustered, named, and organized through the same workflow as people.

## Chunked AI design

Do not put the archive or all subjects in one model context.

1. Detect faces and pet subjects, assigning stable source/region IDs.
2. Generate local face embeddings and temporary review manifests in chunks of roughly 500–1,000 images.
3. Present 20–50 cropped subjects or pair questions per AI context.
4. Merge pairwise results through the local embedding/cluster graph.
5. Make cluster contact sheets containing representative subjects for naming.
6. Require human confirmation before writing a person’s or pet’s name.

Embeddings and manifests are disposable working data. The authoritative identity result is written to image metadata; no permanent identity database is required.

## Learning loop

The pass should produce three training sets:

- positive same-subject pairs (person/person or pet/pet)
- negative different-subject pairs
- hard negatives: not-a-subject/false-positive regions
- subject-type examples: person, pet, and other living subject

Use reviewed examples to calibrate thresholds, retrain or fine-tune the subject detector/filter model, and rerun detection to find missed human and pet subjects. Later passes must report new detections separately from previously reviewed regions.

## Metadata targets

After confirmation, write standard XMP fields where possible:

- `XMP-iptcExt:PersonInImage` for confirmed human names
- `XMP-mwg-rs:RegionName` for a named human or pet region
- `XMP-mwg-rs:RegionType=Face` or `Pet` where region support exists
- `XMP-dc:Subject` / hierarchical subject tokens such as `person:Alex` or `pet:Sam`
- `XMP-iptcExt:PersonInImage` must not be misused for pet names; pet identity belongs in subject/hierarchical metadata

Face count/presence and identity labels remain distinct. Keep an audit manifest containing source hash, region ID, old values, new values, reviewer decision, model version, and timestamp. Writes must be resumable and reversible.

## Safety and acceptance gates

- Read-only detection and review first; metadata writing is separate.
- Never infer identity from a single uncertain match.
- Never label a pet or other animal as a person; classify subject type first. Never label a non-subject visual false positive as either a person or a pet.
- Require exact source hashes before applying a queued write.
- Re-open and verify metadata after writing.
- Keep personal face data and review reports local.
- Do not add this pass to `run all` until review, threshold calibration, and rollback tests pass.

Planned command shape:

```text
bin/photosweep run face-identity ROOT OUTDIR
bin/photosweep identity-review OUTDIR/pairs.jsonl
bin/photosweep identity-apply OUTDIR/approved.jsonl
```

These commands are documentation targets, not implemented production CLI commands yet.
