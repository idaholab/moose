#!/usr/bin/env python3
# * This file is part of the MOOSE framework
# * https://mooseframework.inl.gov
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html
"""
Sync check for the hybrid citation system: every citation key registered in C++
(via addAppCitation) must also exist as an entry in a documentation .bib file,
so the inline BibTeX stays tied to a real, documented reference. Exits non-zero
(listing the offenders) if any registered key is missing from the .bib files.
"""

import os
import re
import sys
import glob
import subprocess

# moose/test/tests/misc/citations/this_file -> moose
MOOSE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../.."))

# Where citations are registered in C++ and where the authoritative .bib entries live
SOURCE_DIRS = [
    os.path.join(MOOSE_DIR, "framework", "src"),
    os.path.join(MOOSE_DIR, "modules"),
]
BIB_GLOBS = [
    os.path.join(MOOSE_DIR, "framework", "doc", "content", "bib", "*.bib"),
    os.path.join(MOOSE_DIR, "modules", "**", "*.bib"),
]

# Each pattern captures the BibTeX key argument of a registration call. \s* spans newlines so the
# patterns match regardless of how clang-format wraps the arguments.
KEY_PATTERNS = [
    re.compile(r'addAppCitation\(\s*"[^"]+"\s*,\s*"([^"]+)"'),
]
CALL_NAMES = ["addAppCitation"]


def registered_keys():
    """Keys registered in C++, as {key: file}."""
    # Limit the (potentially expensive) reads to files that contain a registration call
    files = set()
    for name in CALL_NAMES:
        for d in SOURCE_DIRS:
            res = subprocess.run(
                ["grep", "-rlI", "--include=*.C", name, d],
                capture_output=True,
                text=True,
            )
            files.update(res.stdout.split())
    keys = {}
    for f in files:
        with open(f, encoding="utf-8") as fid:
            content = fid.read()
        for pattern in KEY_PATTERNS:
            for key in pattern.findall(content):
                keys.setdefault(key, f)
    return keys


def bib_keys():
    """All keys defined in the documentation .bib files."""
    keys = set()
    for pattern in BIB_GLOBS:
        for bib in glob.glob(pattern, recursive=True):
            with open(bib, encoding="utf-8", errors="ignore") as fid:
                for m in re.finditer(
                    r"^@\w+\s*\{\s*([^,\s]+)", fid.read(), re.MULTILINE
                ):
                    keys.add(m.group(1))
    return keys


def main():
    reg = registered_keys()
    if not reg:
        print(
            "ERROR: found no registered citations to check; the patterns may be out of date."
        )
        return 1

    documented = bib_keys()
    missing = {key: f for key, f in reg.items() if key not in documented}
    if missing:
        print(
            "ERROR: the following citation keys registered in C++ are not defined in any "
            "documentation .bib file:"
        )
        for key, f in sorted(missing.items()):
            print(f"  '{key}' (registered in {os.path.relpath(f, MOOSE_DIR)})")
        return 1

    print(f"OK: all {len(reg)} registered citation keys are defined in the .bib files.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
