#!/usr/bin/env python3
"""Record active West dependencies; inactive groups need not be cloned."""
import sys

import yaml
from west.manifest import Manifest, ManifestProject


def freeze(manifest):
    data = manifest.as_dict()
    projects = []
    for project in manifest.projects:
        if isinstance(project, ManifestProject) or not manifest.is_active(project):
            continue
        if not project.is_cloned():
            raise ValueError(f"active dependency is not cloned: {project.name}")
        entry = project.as_dict()
        entry["revision"] = project.sha("HEAD")
        projects.append(entry)
    data["manifest"]["projects"] = projects
    return data


if __name__ == "__main__":
    yaml.safe_dump(freeze(Manifest.from_topdir()), sys.stdout, sort_keys=False)
