# Album-art screensaver

A fullscreen slideshow of high-resolution album covers from the music drive,
used as the Omarchy screensaver. Each monitor shows its own shuffled order;
the image changes every 60 seconds with a slow crossfade, over a blurred,
darkened copy of the cover. Any key, click, scroll or mouse movement closes it.

## Layout
- `art/` — the covers (copies; `art/SOURCES.tsv` says where each came from).
- `app/` — the viewer (C++ host + `Screensaver.qml`), built with qmake6.
- `bin/launch-screensaver` — starts one viewer per monitor, the way Omarchy's
  own launcher does. Installed as `~/bin/omarchy-launch-screensaver`, which is
  ahead of `/usr/share/omarchy/bin` on PATH, so Omarchy's idle timer and menu
  use it. Falls back to Omarchy's screensaver if the binary or art is missing.
- `scripts/` — how the art was collected (read-only on the drive):
  `probe.sh` (embedded art sizes), `match_inbox.py` (links Inbox images to
  Library albums through `Manifest.db`), `candidates.py`, `copy_covers.py`.

## Build
    cd app && qmake6 screensaver.pro -o ../build/Makefile && make -C ../build

## Use
- Start now: `omarchy-launch-screensaver force`
- Starts on its own after Omarchy's idle time (`idle.screensaver` in
  `~/.config/omarchy/shell.json`, currently 150 s) unless "stay awake" is on.
- Settings: `~/.config/album-art-screensaver.conf` with `INTERVAL=60` and/or
  `ART_DIR=/some/folder`.
- Undo: `rm ~/bin/omarchy-launch-screensaver` (Omarchy's own comes back).

## Not in this repo
`art/` and `data/` are local only (git-ignored): the covers are the artists'
work, and `data/` holds listings of the music drive. Rebuild `art/` with the
scripts above.
