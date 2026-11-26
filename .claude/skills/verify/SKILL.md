---
name: verify
description: Run the real album-art screensaver offscreen and see it working, without touching the owner's display, art or build. Use before calling any change to the viewer, its QML or its build done, and to gather the evidence for its PR.
---

# Verify the real screensaver

`scripts/verify [interval]` builds the viewer into a temporary folder, makes three stand-in covers, and runs it offscreen at real speed (interval 5 s by default), grabbing a frame while each slide is up. Evidence goes to `.scratch/verify/<time>/`: `frame-NN.png`, `app.log`, and `LEDGER.md` with the slides shown in order.

1. **Make it able to fail.** Where you can, run it on `main` first (in a worktree) and see the problem, then on your change and see it gone.
2. **Run it** in the foreground. It exits non-zero if the app fails, a frame is missing, or the first three slides aren't the three stand-ins, each once.
3. **Look** at the frames that matter with Read, and at `LEDGER.md`.
4. **Report** in the PR's Evidence: the command, the slides shown, and the evidence folder.

## What it can't show

The cover and its blurred backdrop are drawn through `MultiEffect`, which the offscreen platform leaves blank: frames show the stage and the captions only. The covers, the crossfade's look, real input closing it, focus handling and the launcher's per-monitor windows stay owner checks on the real display; say so in the PR when a change touches them.
