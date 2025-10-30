# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Configures pytest for the moosecontrol module."""

import os
import shutil
import subprocess
from typing import Optional, Tuple

import pytest


def pytest_addoption(parser):
    """Add custom options to pytest."""
    parser.addoption(
        "--no-moose",
        action="store_true",
        default=False,
        help="Skip tests that require a moose executable",
    )
    parser.addoption(
        "--moose-exe",
        type=str,
        help="Specify the path to the moose executable instead of searching",
    )


def find_moose_test_exe() -> Tuple[Optional[str], list[str]]:
    """
    Try to find the moose_test executable in PATH or in-tree.

    Returns
    -------
    Optional[str]:
        The absolute executable path, if found.
    list[str]:
        The paths that were searched.

    """
    this_dir = os.path.dirname(__file__)
    test_dir = os.path.abspath(os.path.join(this_dir, "..", "..", "test"))
    if not os.path.isdir(test_dir):
        test_dir = None

    found: Optional[str] = None
    searched: list[str] = []

    for method in ["dbg", "devel", "oprof", "opt"]:
        exe = f"moose_test-{method}"

        # Exists in PATH
        searched.append(exe)
        if which := shutil.which(exe):
            found = which
            break

        # Exists in tree
        if test_dir is not None:
            in_tree = os.path.join(test_dir, exe)
            searched.append(in_tree)
            if os.path.exists(in_tree):
                found = in_tree
                break

    return found, searched


def pytest_configure(config):
    """
    Configure the tests; ran once before all tests.

    If --no-moose is not set, make sure that we have
    a working MOOSE executable. If not, automatically
    fail the tests that depend on MOOSE. If so, make
    the executable path available to tests.
    """
    config.moose_exe = None

    # --no-moose not set, running with MOOSE
    if not config.getoption("--no-moose"):
        # Paths searched; used in error handling
        searched: Optional[list[str]] = None

        moose_exe = config.getoption("--moose-exe")
        # User set --moose-exe, make sure it exists
        if moose_exe is not None:
            if not shutil.which(moose_exe):
                pytest.exit(f'--moose-exe: Executable "{moose_exe}" not found')
        # --moose-exe not set, do a search
        else:
            moose_exe, searched = find_moose_test_exe()

        # Nothing found
        if moose_exe is None:
            message = "Failed to find a MOOSE executable.\n\n"
            if searched:
                prefix = "\n  - "
                message += f"Searched paths:{prefix}{prefix.join(searched)}\n\n"
            message += "Either disable moose tests with --no-moose, "
            message += "or specify an exectuable with --moose-exe."
            pytest.exit(message)

        # Make sure it runs
        cmd = [moose_exe, "--help"]
        process = subprocess.run(
            cmd,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
        if process.returncode != 0:
            pytest.exit(
                "Failed to run MOOSE executable with "
                f"\"{' '.join(cmd)}\":\n\n{process.stdout}"
            )

        print(f"INFO: Using moose executable {moose_exe}")
        # Set in shared state to be used as a fixture
        config.moose_exe = moose_exe
    # Not using moose
    else:
        print("INFO: Not running tests that require moose")


def pytest_collection_modifyitems(config, items):
    """Add custom skips to pytest."""
    # Skip 'moose' tests if --no-moose is set
    if config.getoption("--no-moose"):
        marker = pytest.mark.skip(reason="--no-moose")
        for item in items:
            if "moose" in item.keywords:
                item.add_marker(marker)


@pytest.fixture
def moose_exe(pytestconfig) -> Optional[str]:
    """Get the moose executable found during init, if any."""
    assert isinstance(pytestconfig.moose_exe, (type(None), str))
    return pytestconfig.moose_exe
