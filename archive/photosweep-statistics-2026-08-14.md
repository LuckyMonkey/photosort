# PhotoSweep statistics

Snapshot taken 2026-08-14 while the Facebook import was still running.

## Source totals

| Source | Handled records | Canonical organized files | Exact duplicates removed | Collision-renamed |
|---|---:|---:|---:|---:|
| Facebook | 12,295 | 5,096 | 7,199 | 0 |
| OneDrive | 16,488 | 10,643 | 5,845 | 2,334 |
| Google Photos | 26,244 | 26,244 | 0 | 580 |
| Other/unclassified | 6 | 6 | 0 | 0 |
| **Total** | **55,033** | **41,989** | **13,044** | **2,914** |

Canonical organized files = `moved` + `collision-renamed`. Exact duplicates are counted as handled but are not retained as a second organized file.

## Classification totals

| Source | Photos | Screenshots | Other images | Total |
|---|---:|---:|---:|---:|
| Facebook | 0 | 763 | 11,532 | 12,295 |
| OneDrive | 9,002 | 1,632 | 5,854 | 16,488 |
| Google Photos | 17,946 | 2,889 | 5,409 | 26,244 |
| Other/unclassified | 0 | 0 | 6 | 6 |
| **Total** | **26,948** | **5,284** | **22,801** | **55,033** |

## Status totals

| Status | Count |
|---|---:|
| Moved | 39,075 |
| Collision-renamed | 2,914 |
| Exact-duplicate-removed | 13,044 |
| Errors | 0 |

Facebook is still processing, so its figures are a lower-bound snapshot and will increase when the run completes. Source data: `Photos/reports/files.tsv`.
