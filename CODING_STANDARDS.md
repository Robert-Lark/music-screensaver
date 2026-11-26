# Coding standards

This project follows the shared standards in `~/Projects/docs/coding-standards.md`: read them before writing or reviewing code. This file says which of them the checks enforce, and adds the project's own rules. Where the two differ, this file wins.

## Enforced by the checks

Reviewers skip these: `scripts/check` enforces them, and its canary proves each still catches a planted violation.

- **C++:** the build runs with `-Wall -Wextra -Werror`.
- **QML:** `qmllint`. `app/Screensaver.qml` reads the context properties `main.cpp` sets, which qmllint counts as warnings inside the `Slide` component; their count sits in `guards-allowlist.json` and only shrinks. qmllint doesn't flag them at the root of the file, so a new context property is a review point, not a check.
- **Shell:** `shellcheck` on the launcher, the scripts and the hook.
- **Python** (the one-off collection scripts): ruff's real-error rules (`F`, `E9`, `B`); their style is left as it is.
- **File size:** at most 5,500 tokens per file under `app/`, `bin/` and `scripts/`.

## This project's rules

- **The real screensaver path stays lean.** It runs for hours on every monitor: no timers or bindings that wake it more than the slideshow needs. Test-only behaviour (`--no-focus-exit`, `--grab`) stays behind its option.
- **The launcher falls back to Omarchy's own screensaver** whenever the binary or the art is missing; a change to it keeps that path working.
