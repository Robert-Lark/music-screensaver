# Album-art screensaver

A fullscreen slideshow of album covers, used as the Omarchy screensaver: `README.md` has the layout, build and use.

## Safety rules

1. **The covers and drive listings stay local.** `art/` and `data/` are git-ignored: the covers are the artists' work and this repo is public. Nothing from them is committed, pasted into an issue or uploaded.
2. **The music drive is read-only, and only the owner runs the collection scripts** (`scripts/probe.sh`, `match_inbox.py`, `candidates.py`, `copy_covers.py`) against it.
3. **The installed screensaver stays as the owner left it.** `build/` is what `~/bin/omarchy-launch-screensaver` runs, and `~/.config/album-art-screensaver.conf` is the owner's: checks and verify runs build into temporary folders and leave both alone. Never start the screensaver on the owner's display; `scripts/verify` runs it offscreen.

## Commands

- Every check: `scripts/check` (`--static` skips the build; the pre-commit hook runs that). `CODING_STANDARDS.md` lists what it enforces.
- See it running: the `verify` skill (`scripts/verify`).
- Turn the hook on in a new clone: `git config core.hooksPath .githooks`.
