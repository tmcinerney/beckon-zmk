#!/usr/bin/env python3
"""Keep West and Nix on the same reviewed, immutable direct dependencies."""
import json
from pathlib import Path
import re
import subprocess
import sys

import yaml


def check(root, resolved=False):
    frozen = {item["name"]: item["revision"] for item in json.loads((root / "nix/manifest.json").read_text())}
    projects = yaml.safe_load((root / "app/west.yml").read_text())["manifest"]["projects"]
    for project in projects:
        name, revision = project["name"], project["revision"]
        if not re.fullmatch(r"[0-9a-f]{40}", revision) or frozen.get(name) != revision:
            raise ValueError(f"{name}: West must use the same immutable revision as nix/manifest.json")
        if resolved:
            path = root / project.get("path", name)
            actual = subprocess.check_output(["git", "-C", str(path), "rev-parse", "HEAD"], text=True).strip()
            if actual != revision:
                raise ValueError(f"{name}: resolved {actual}, expected {revision}")
    print("West/Nix dependency pins agree" + ("; fetched revisions verified" if resolved else ""))


if __name__ == "__main__":
    try:
        check(Path(__file__).resolve().parents[2], "--resolved" in sys.argv[1:])
    except ValueError as error:
        sys.exit(str(error))
