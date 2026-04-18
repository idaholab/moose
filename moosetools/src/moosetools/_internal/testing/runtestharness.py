# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implement a helper for running the MOOSE TestHarness."""

import json
import os
import shutil
from contextlib import redirect_stdout
from dataclasses import dataclass
from io import StringIO
from tempfile import TemporaryDirectory
from typing import Iterable, Optional

from TestHarness import TestHarness

MOOSE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../../.."))


@dataclass
class TestHarnessResult:
    """Structure containing a test harness result."""

    # On screen output
    output: str = ""
    # JSON results
    results: Optional[dict] = None
    # TestHarness that was ran
    harness: Optional[TestHarness] = None
    # The exit code (if any)
    exit_code: Optional[int] = None


def run_test_harness(
    moose_exe: str,
    test_dir: str,
    spec_file: str,
    harness_args: Optional[Iterable[str]] = None,
    minimal_capabilities: bool = True,
    run: bool = True,
) -> TestHarnessResult:
    """
    Run the MOOSE TestHarness.

    Will copy the contents from the test folder into a
    temporary folder to run it in a way that is stateless.

    Arguments:
    ---------
    moose_exe : str
        Path to the MOOSE executable.
    test_dir : str
        Path to the test directory to copy tests from.
    spec_file : str
        Name of the test specification file in the test directory.

    Additional Arguments
    --------------------
    harness_args : Optional[Iterable[str]]
        Command line arguments to pass to the test harness.
    minimal_capabilities : bool
        Whether or not to pass --minimal-capabilities; default is True.
    run : bool
        Whether to run the tests (False just constructs); default is True.

    """
    result = TestHarnessResult()

    assert os.path.isfile(moose_exe)
    print(os.path.abspath(test_dir))
    assert os.path.isdir(test_dir)

    argv = (
        ["unused"]
        + (list(harness_args) if harness_args else [])
        + ["--term-format", "njCst", "-i", spec_file]
    )
    if minimal_capabilities:
        argv += ["--minimal-capabilities"]

    with TemporaryDirectory() as dir:
        # Copy test contents into directory
        shutil.copytree(test_dir + "/", dir, dirs_exist_ok=True)

        # Link the executable so that it can be found
        os.symlink(moose_exe, os.path.join(dir, os.path.basename(moose_exe)))

        # Move into the test directory
        cwd = os.getcwd()
        os.chdir(dir)

        # For capturing on-screen output
        stdout = StringIO()

        try:
            with redirect_stdout(stdout):
                result.harness = TestHarness.build(
                    argv, "moose_test", MOOSE_DIR, skip_testroot=True
                )
                if run:
                    result.harness.findAndRunTests()
        finally:
            os.chdir(cwd)
            result.output = stdout.getvalue()
            stdout.close()

        # Load results file if it is available
        results_file = result.harness.options.results_file
        if os.path.isfile(results_file):
            with open(results_file, "r") as f:
                result.results = json.loads(f.read())

        return result
