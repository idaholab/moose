#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Command line helper for converting test spec params to capabilities."""

import argparse
import os
import subprocess
import sys
import traceback
from contextlib import suppress
from typing import Optional

# Add MOOSE python to path
MOOSE_DIR = os.environ.get(
    "MOOSE_DIR",
    os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), "..")),
)
sys.path.append(os.path.join(MOOSE_DIR, "python"))

# pyhit here needs to be imported first so that it adds
# the hit lib to path
import pyhit  # noqa: I001 E402
import hit  # noqa: E402


class SpecModifier:
    """Modifies a single test specification file."""

    def __init__(self, file: str):
        """
        Initialize state.

        Arguments:
        ---------
        file : str
            The path to the test spec.

        """
        # Path to the file
        self.file: str = file
        # Whether or not we've modified the file
        self.modified: bool = False

    def recurse(self, node: pyhit.Node):
        """Recursively find a single spec, fix it, and fix all of its children."""
        for child in node:
            if child.get("type") is not None:
                if self.modify_test(child):
                    self.modified = True
            else:
                self.recurse(child)

    @staticmethod
    def modify_test(node: pyhit.Node) -> bool:
        """
        Modify a test spec, changing old options to capabilities.

        Arguments:
        ---------
        node : pyhit.Node
            The node that represents the HIT tree for the test spec.

        Returns:
        -------
            bool : Whether or not the spec was modified.

        """
        assert node.get("type") is not None

        # Options that we've converted to capabilities
        new_capabilities: list[str] = []
        # Comments to append, accumulated from other options
        capability_comments: list[str] = []
        # Params to drop
        drop_params: set[str] = set()
        # Where to insert the capability param if one doesn't exist
        insert_index: Optional[int] = None

        def fix_param(param: str, get_capabilities):
            nonlocal insert_index
            value = node.get(param)
            if value is None:
                return
            capabilities = get_capabilities(value)
            if not capabilities:
                return
            if " " in capabilities:
                capabilities = f"({capabilities})"
            new_capabilities.append(capabilities)
            if comment := node.comment(param):
                capability_comments.append(comment)
            if insert_index is None:
                insert_index = 0
                for child in node.__dict__["_Node__hitnode"].children():
                    insert_index += 1
                    if child.path() == param:
                        break
            drop_params.add(param)

        # [min,max]_[parallel,threads]: not enabled yet
        # for suffix in ["parallel", "threads"]:
        #     capability = "mpi_procs" if suffix == "parallel" else "num_threads"
        #     min_name = f"min_{suffix}"
        #     max_name = f"max_{suffix}"
        #     min_value = node.get(min_name)
        #     max_value = node.get(max_name)
        #     if min_value is not None:
        #         if max_value is not None and int(min_value) == int(max_value):
        #             fix_param(min_name, lambda value: f"{capability}={value}")
        #             drop_params.add(max_name)
        #         elif int(min_value) == 1:
        #             drop_params.add(min_name)
        #     if min_name not in drop_params:
        #         fix_param(min_name, lambda value: f"{capability}>={value}")
        #     if max_name not in drop_params:
        #         fix_param(
        #             max_name,
        #             lambda value: (
        #                 f"{capability}<={value}"
        #                 if int(value) > 1
        #                 else f"{capability}=1"
        #             ),
        #         )
        # min_ad_size
        fix_param("min_ad_size", lambda value: f"ad_size>={value}")
        # max_ad_size
        fix_param("max_ad_size", lambda value: f"ad_size<={value}")
        # valgrind: not enabled yet
        # valgrind = node.get("valgrind")
        # if valgrind is not None:
        #     if valgrind.lower() == "none":
        #         fix_param("valgrind", lambda value: "!valgrind")
        #     else:
        #         fix_param("valgrind", lambda value: f"valgrind={valgrind.lower()}")
        # *_version
        versioned_params = ["petsc", "slepc", "exodus", "vtk", "libtorch"]
        for param in versioned_params:
            param_name = f"{param}_version"

            def do_versioned_param(value):
                conditions = []
                value_split = value.split()
                for entry in value_split:
                    if entry == "&&":
                        conditions.append("&")
                    elif entry == "||":
                        conditions.append("|")
                    else:
                        conditions.append(f"{param}{entry}")
                return " ".join(conditions)

            fix_param(param_name, do_versioned_param)
        # string params, implicit or, negation allowed
        string_params = [
            "compiler",
            "method",
            "platform",
            "machine",
            "library_mode",
            "installation_type",
        ]
        for param in string_params:

            def do_string_param(value):
                conditions = []
                for split_value in value.split(" "):
                    prefix = ""
                    if split_value[0] == "!":
                        prefix = "!"
                        split_value = split_value[1:]
                    conditions.append(f"{param}{prefix}={split_value.lower()}")
                return " | ".join(conditions)

            fix_param(param, do_string_param)
        # boolean params
        bool_params = [
            "unique_id",
            "tecplot",
            "vtk",
            "petsc_debug",
            "superlu",
            "mumps",
            "strumpack",
            "slepc",
            "parmetis",
            "chaco",
            "party",
            "ptscotch",
            "libpng",
            "libtorch",
            "mfem",
            "hpc",
            # Not enabled yet
            # "recover",
            # "restep",
            # "heavy",
        ]
        for param in bool_params:

            def do_bool_param(value):
                bool_value = None
                if isinstance(value, bool):
                    bool_value = value
                elif isinstance(value, str):
                    bool_value = value.strip().lower() == "true"
                else:
                    return None
                prefix = "!" if not bool_value else ""
                return f"{prefix}{param}"

            fix_param(param, do_bool_param)
        # numeric params
        numeric_params = ["dof_id_bytes"]
        for param in numeric_params:
            fix_param(param, lambda value: f"{param}={value}")
        # fparse_jit
        fix_param("fparser_jit", lambda value: "fparser=jit")

        # required_applications
        def do_required_applications(value):
            conditions = []
            for value_split in value.split(" "):
                conditions.append(value_split.lower())
            return " & ".join(conditions)

        fix_param("required_applications", do_required_applications)

        # Nothing to do
        if not new_capabilities:
            return False

        # Update capabilities string
        capabilities = node.get("capabilities", "")
        assert isinstance(capabilities, str)
        if capabilities:
            capabilities += " & "
        capabilities += " & ".join(new_capabilities)
        capabilities = f"'{capabilities}'"

        # Insert as a new node, in the position of one
        # of the old (to be removed) parameter nodes
        if node.get("capabilities") is None:
            assert isinstance(insert_index, int)
            hitnode = node.__dict__["_Node__hitnode"]
            new = hit.NewField("capabilities", hit.FieldKind.String, capabilities)
            hitnode.insertChild(insert_index - 1, new)
        # Update the current capabilities string
        else:
            node["capabilities"] = capabilities

        # Drop the params no longer needed AFTER possibly
        # adding a new capability param (where the ordering
        # needs to depend on the not-yet-removed params)
        for param in drop_params:
            node.removeParam(param)

        # Add to comment if set
        if capability_comments:
            comment = node.comment("capabilities")
            if comment:
                capability_comments = [comment] + capability_comments
            node.setComment("capabilities", "; ".join(capability_comments))

        # Did something
        return True

    def run(self, dry_run: bool) -> bool:
        """
        Runner that modifies a test spec.

        Returns whether or not the spec was modified.
        """
        root = pyhit.load(self.file)

        # Find [Tests] node
        tests_node = None
        for child in root:
            if child.name.lower() == "tests":
                tests_node = child
                break

        # Nothing to do
        if tests_node is None:
            return False

        # Recurse, modifying each test
        self.recurse(tests_node)
        # Nothing changed
        if not self.modified:
            return False

        # Overwrite changes
        if not dry_run:
            pyhit.write(self.file, root)
        return True


def parse_args() -> argparse.Namespace:
    """Parse arguments."""
    parser = argparse.ArgumentParser(description="Converts test specs to capabilities")
    parser.add_argument("root_dir", type=str, help="Root directory to start the search")
    parser.add_argument(
        "--spec-file",
        type=str,
        default="tests",
        help="Name of the spec file (default: tests)",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        default=False,
        help="Perform a check; do not make changes and exit nonzero if changes needed",
    )
    return parser.parse_args()


def main():
    """Run the program."""
    args = parse_args()
    root_dir = os.path.abspath(args.root_dir)
    check = args.check

    if not os.path.exists(root_dir):
        print(f"ERROR: Root directory {root_dir} does not exist")
        sys.exit(1)

    skip_dirs = []

    # See if we're in a git repository so we can skip checking submodules
    git_root = None
    with suppress():
        git_root = subprocess.check_output(
            ["git", "rev-parse", "--show-toplevel"], cwd=root_dir, text=True
        ).strip()

    # In a git repository, skip checking .git directory and all submodules
    if git_root:
        skip_dirs.append(os.path.join(git_root, ".git"))
        gitmodules = subprocess.check_output(
            [
                "git",
                "config",
                "--file",
                ".gitmodules",
                "--name-only",
                "--get-regexp",
                "path",
            ],
            cwd=git_root,
            text=True,
        ).strip()
        for entry in gitmodules.splitlines():
            skip_dirs.append(os.path.join(git_root, "".join(entry.split(".")[1:-1])))

    test_specs = []
    for dirpath, _, filenames in os.walk(root_dir, followlinks=True):
        # Directory is skipped
        if [d for d in skip_dirs if dirpath.startswith(d)]:
            continue

        for filename in filenames:
            path = os.path.abspath(os.path.join(dirpath, filename))
            if path.endswith(f"/{args.spec_file}"):
                test_specs.append(path)

    print(f"Found {len(test_specs)} test specs in {os.path.abspath(args.root_dir)}")
    num_modified = 0
    num_failed = 0
    for path in test_specs:
        path = os.path.relpath(path)
        modified = False
        try:
            modified = SpecModifier(path).run(check)
        except Exception:
            print(f"FAILED {path}")
            print(traceback.format_exc())
            num_failed += 1
        if modified:
            print("CHANGES REQUIRED" if check else "MODIFIED", path)
            num_modified += 1
    if check:
        print(f"{num_modified} test specs require changes")
        if num_modified:
            sys.exit(1)
    else:
        print(f"Modified {num_modified} test specs, {num_failed} failed modifications")


if __name__ == "__main__":
    main()
