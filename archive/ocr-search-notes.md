# Lightweight local OCR search

Recoll 1.36.1 indexes the configured roots into Xapian. Tesseract OCR output is cached under `~/.cache/recoll-ocr-search/ocrcache`; originals are never modified and no sidecars are created. Edit `indexed-locations.conf`, then rerun `./install.sh` to change roots.

Formats: JPEG/JPG, PNG, TIFF/TIF, BMP, WEBP, and image-only PDFs. Images below 160x80 or 40,000 pixels are skipped. English is configured. Hidden/cache/development/Steam/Trash paths are excluded.

Commands: `ocr-search "phrase"`, `ocr-index-status`, `ocr-reindex PATH`, `ocr-search-pause`, `ocr-search-resume`. Service: `systemctl --user status recoll-ocr-monitor.service`. Logs: `journalctl --user -u recoll-ocr-monitor.service`. Safe full rebuild: pause, run `nice -n 15 ionice -c 3 recollindex -c ~/.recoll -z`, then resume.

The index/cache is ~/.cache/recoll-ocr-search. The uninstall script removes only setup-created configuration, service, helpers, shortcut, OCR cache, and Xapian index; timestamped backups and original indexed files remain untouched.

XFCE shortcut: Ctrl+Super+F. Super+F remains assigned to the file manager. Recoll uses the default MIME application when opening the original result; stored text enables snippets/previews.
