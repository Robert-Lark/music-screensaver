"""Find loose images that belong to an album filed under Dark Ambient or Drone.

An image belongs to an album when the nearest folder above it that held audio
(per the Manifest) sent all its audio to that one Library album folder.
Mixed folders (e.g. loose tracks at the top of Downloads) are skipped.
"""
import os, re, sqlite3, sys, json, collections
M = "/run/media/horse/Music"
STYLES = ("Dark Ambient", "Drone")
db = sqlite3.connect(f"file:{M}/Manifest.db?mode=ro&immutable=1", uri=True)
dest = {}
for op, ip, lp in db.execute("select original_path, inbox_path, library_path from placements"):
    dest[op] = (ip, lp)
for op, to in db.execute("select original_path, to_path from rehomes"):
    if op in dest: dest[op] = (dest[op][0], to)
audio_dirs = collections.defaultdict(set)  # inbox folder -> library album folders
for op, kind, ip in db.execute("select original_path, kind, inbox_path from files where kind='audio'"):
    lp = dest.get(op, (ip, ""))[1]
    audio_dirs[os.path.dirname(ip)].add(os.path.dirname(lp) if lp.startswith("Library/") else "(not in library)")
rx = re.compile(r"^(\d+)t(\d+)t[^t]*Bt(.*)$")
out, skipped_mixed = [], 0
for l in open("data/loose.tsv"):
    m = rx.match(l.rstrip("\n"))
    if not m: continue
    w, h, p = int(m[1]), int(m[2]), m[3]
    d = os.path.dirname(os.path.relpath(p, M))
    while d and d not in audio_dirs: d = os.path.dirname(d)
    if not d: continue
    albums = audio_dirs[d]
    if len(albums) != 1: skipped_mixed += 1; continue
    (alb,) = albums
    parts = alb.split("/")
    if len(parts) > 2 and parts[2] in STYLES:
        out.append((min(w, h), w, h, p, alb))
json.dump(out, open("data/dad_loose.json", "w"), indent=0)
print("images:", len(out), "albums:", len({o[4] for o in out}), "(skipped in mixed folders:", skipped_mixed, ")", file=sys.stderr)
for t in (1000, 1400, 1500, 2000, 3000):
    print(f">= {t}px short side:", sum(1 for o in out if o[0] >= t), "albums:", len({o[4] for o in out if o[0] >= t}), file=sys.stderr)
