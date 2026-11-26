"""Copy the chosen covers off the music drive (read-only) into art/, named 'Artist - Album'."""
import json, os, shutil, subprocess, hashlib, sys
DROP = {2,3,4,5,6,12,19,22,23,24,25,26,33,34,41,52,53,54,55,57,58,73,74,75,86}
SUFFIX = {1: " (2nd Ed)"}
cs = json.load(open("data/candidates.json"))
os.makedirs("art", exist_ok=True)
rows = []
for c in cs:
    if c["id"] in DROP: continue
    artist, album = c["album"].split("/")[-2:]
    src = c["src"]
    name = f"{artist} - {album}{SUFFIX.get(c['id'], '')}".replace("/", "_")
    if src.startswith("embedded:"):
        dst = f"art/{name}.jpg"
        subprocess.run(["ffmpeg", "-v", "error", "-n", "-i", src[9:], "-map", "0:v:0", "-c", "copy", dst], check=True)
    else:
        dst = f"art/{name}{os.path.splitext(src)[1].lower()}"
        assert not os.path.exists(dst), dst
        shutil.copy2(src, dst)
        assert hashlib.sha1(open(src,"rb").read()).digest() == hashlib.sha1(open(dst,"rb").read()).digest()
    rows.append(f"{os.path.basename(dst)}\t{c['w']}x{c['h']}\t{src}")
open("art/SOURCES.tsv", "w").write("file\tsize\tsource\n" + "\n".join(sorted(rows)) + "\n")
print(len(rows), "copied", file=sys.stderr)
