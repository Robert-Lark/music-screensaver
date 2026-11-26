"""List unique Dark Ambient / Drone images >= 1500px on the short side (loose + embedded)."""
import json, hashlib, os, sys
MIN = 1500
out = json.load(open("data/dad_loose.json"))
for l in open("data/dad.tsv"):
    d, p = l.rstrip("\n").split("\t", 1)
    if d == "none": continue
    w, h = map(int, d.split("x"))
    if min(w, h) >= MIN: out.append((min(w, h), w, h, "embedded:" + p, os.path.relpath(os.path.dirname(p), "/run/media/horse/Music")))
seen, cands = set(), []
for s, w, h, p, alb in sorted(out, key=lambda o: (o[4], o[3])):
    if s < MIN: continue
    key = p if p.startswith("embedded:") else hashlib.sha1(open(p, "rb").read()).hexdigest()
    if p.startswith("embedded:"):
        if alb in {c["album"] for c in cands if c["src"].startswith("embedded:")}: continue
    elif key in seen: continue
    seen.add(key)
    cands.append({"id": len(cands), "w": w, "h": h, "src": p, "album": alb})
json.dump(cands, open("data/candidates.json", "w"), indent=1)
print(len(cands), "candidates,", len({c["album"] for c in cands}), "albums", file=sys.stderr)
