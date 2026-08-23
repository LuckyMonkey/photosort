#!/bin/sh
set -eu

# Build read-only smart-folder views from the private geo-normalize TSV.
# Usage: build-location-smart-folders.sh GEO_TSV DEST_ROOT [STATE_ICON_DIR]
geo_tsv=${1:?geo TSV required}
dest_root=${2:?destination root required}
icon_root=${3:-}
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
state_list=${STATE_LIST:-$script_dir/state-folders.txt}
locations="$dest_root/LOCATIONS"
mkdir -p "$locations"

# Seed useful state folders even before the first matching GPS record.
if [ -f "$state_list" ]; then
    while IFS='|' read -r seed_state seed_code; do
        case "$seed_state" in ""|\#*) continue ;; esac
        folder="$locations/$seed_state"
        mkdir -p "$folder"
        if [ -n "$icon_root" ] && [ -f "$icon_root/$seed_code.svg" ]; then
            ln -sfn -- "$icon_root/$seed_code.svg" "$folder/.state.svg"
            printf '[Desktop Entry]\nType=Directory\nIcon=.state.svg\n' > "$folder/.directory"
        fi
    done < "$state_list"
fi

# geo-normalize-exif.py columns: source latitude longitude place city state state_code ...
tab=$(printf "\t")
tail -n +2 "$geo_tsv" | while IFS="$tab" read -r source latitude longitude place city state state_code rest; do
    [ -n "$source" ] || continue
    [ -n "$state" ] || state="Unknown"
    safe_state=$(printf '%s' "$state" | tr '/:' '__' | sed 's/[[:space:]]\+/ /g; s/^ *//; s/ *$//')
    [ -n "$safe_state" ] || safe_state="Unknown"
    folder="$locations/$safe_state"
    mkdir -p "$folder"
    base=${source##*/}
    link="$folder/$base"
    if [ -e "$link" ] || [ -L "$link" ]; then
        digest=$(sha256sum "$source" | cut -c1-8)
        link="$folder/${base%.*}--$digest.${base##*.}"
    fi
    ln -s -- "$source" "$link"
    if [ -n "$icon_root" ] && [ -f "$icon_root/$state_code.svg" ]; then
        ln -sfn -- "$icon_root/$state_code.svg" "$folder/.state.svg"
        printf '[Desktop Entry]\nType=Directory\nIcon=.state.svg\n' > "$folder/.directory"
    fi
done
