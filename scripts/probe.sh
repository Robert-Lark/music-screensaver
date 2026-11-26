#!/bin/bash
# For each audio file: print width<TAB>height<TAB>path of its first embedded picture (blank if none).
for f in "$@"; do
  wh=$(ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=p=0:s=x "$f" 2>/dev/null | head -1)
  printf '%s\t%s\n' "${wh:-none}" "$f"
done
