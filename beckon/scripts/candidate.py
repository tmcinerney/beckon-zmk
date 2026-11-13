#!/usr/bin/env python3
"""Package only a checked build, retaining its original source identity."""
import hashlib
import json
from pathlib import Path
import re
import shutil
import sys


def prepare(directory, release_tag, candidate_tag):
    if not re.fullmatch(r"v[0-9]+\.[0-9]+\.[0-9]+", release_tag):
        raise ValueError("release tag must be vMAJOR.MINOR.PATCH")
    if not re.fullmatch(re.escape(release_tag) + r"-rc\.[0-9]+", candidate_tag):
        raise ValueError("candidate tag must match RELEASE_TAG-rc.NUMBER")
    commit = json.loads((directory / "source.json").read_text())["commit"]
    if not re.fullmatch(r"[0-9a-f]{40}", commit):
        raise ValueError("build source commit must be a full SHA")
    # Reject incomplete builds before generating any publishable metadata.
    for name in ("glove80.uf2", "build.log", "west-frozen.yml"):
        if not (directory / name).stat().st_size:
            raise ValueError(f"missing or empty build asset: {name}")
    firmware = (directory / "glove80.uf2").read_bytes()
    if len(firmware) % 512 or firmware[:8] != bytes.fromhex("5546320a57515d9e"):
        raise ValueError("firmware is not a UF2 image")
    shutil.copyfile(directory / "glove80.uf2", directory / "beckon.uf2")
    digest = hashlib.sha256(firmware).hexdigest()
    (directory / "SHA256SUMS").write_text(f"{digest}  beckon.uf2\n")
    metadata = {"release_tag": release_tag, "candidate_tag": candidate_tag, "commit": commit, "sha256": digest}
    (directory / "candidate.json").write_text(json.dumps(metadata, indent=2) + "\n")
    smoke = Path(__file__).resolve().parents[1] / "release/smoke-test.md"
    shutil.copyfile(smoke, directory / "SMOKE-TEST.md")
    return metadata


if __name__ == "__main__":
    try:
        prepare(Path(sys.argv[1]), sys.argv[2], sys.argv[3])
    except (ValueError, KeyError, OSError) as error:
        sys.exit(str(error))
