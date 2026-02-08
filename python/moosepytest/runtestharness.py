# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import os
import shutil
import json

from contextlib import redirect_stdout
from dataclasses import dataclass
from tempfile import TemporaryDirectory
from typing import Optional, Iterable
from io import StringIO

from TestHarness import TestHarness


MOOSE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))


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
    # tmp_output: bool = True,
    minimal_capabilities: bool = True,
    # capture_results: bool = True,
    exit_code: int = 0,
    run: bool = True,
    # tests: Optional[dict[str, dict]] = None,
) -> TestHarnessResult:
    """
    Helper for running tests.

    Args:
        Command line arguments to pass to the test harness
    Keyword arguments:
        tmp_output: Set to store output separately using the -o option in a temp directory
        minimal_capabilities: Set to run with minimal capabilities (--minimal-capabilities)
        capture_results: Set to capture on-screen results
        exit_code: Exit code to check against
        tests: Test spec (dict of str -> params) to run instead

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
        except SystemExit as e:
            if isinstance(e.code, int):
                result.exit_code = e.code
                return result
            raise
        finally:
            os.chdir(cwd)
            result.output = stdout.getvalue()
            stdout.close()

        # Load results file if it is available
        results_file = result.harness.options.results_file
        if os.path.isfile(results_file):
            with open(results_file, "r") as f:
                result.results = json.loads(f.read())

        # If we've gotten this far, we exited zero
        result.exit_code = result.harness.error_code

        return result
