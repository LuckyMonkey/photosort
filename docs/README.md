# Documentation index

This directory is segmented by the question an operator or developer is trying to answer.

## Operator path

1. [Production demo](production-demo.md) — build, preflight, run, and stop safely.
2. [Architecture](architecture.md) — understand the stages and report contracts.
3. Choose a sweep: [OCR](sweeps/ocr.md), [faces](sweeps/faces.md), [GPS/GIS](sweeps/gps.md), or [swatch/duplicates](sweeps/swatch.md).
4. [Duplicate review](duplicate-review.md) — inspect suggestions without deleting anything.
5. [Operations](operations.md) — privacy, retention, retries, and incident handling.

## Developer path
2. [Function reference](function-reference.md) — map each sweep call and its safety reason.

1. [Development](development.md) — build flags, checks, portability, and contribution rules.
2. [System inventory](system-inventory.md) — what came from the workstation and what remains an external dependency.
3. [Rules](../rules/photosweep.yaml) — thresholds and safety policy.

## Historical path

The original manual PHP sorter, sample images, and screenshots are in [`../archive/legacy-photosort/`](../archive/legacy-photosort/). The archive is explanatory and is not built by the C Makefile.
