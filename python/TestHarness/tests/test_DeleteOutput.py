# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
import tempfile

from TestHarnessTestCase import MOOSE_DIR, MOOSE_PYTHON, TestHarnessTestCase

TESTER_DIR = os.path.join(MOOSE_PYTHON, "TestHarness", "testers")
if TESTER_DIR not in sys.path:
    sys.path.append(TESTER_DIR)

from CheckFiles import CheckFiles


def _make_check_files_params(test_dir):
    params = CheckFiles.validParams()
    params["type"] = "CheckFiles"
    params["test_name"] = "test_dir.test_name"
    params["test_name_short"] = "test_name"
    params["command"] = "/usr/bin/touch"
    params["check_files"] = ["cleanup.txt"]
    params["moose_dir"] = MOOSE_DIR
    params["moose_python_dir"] = MOOSE_PYTHON
    params["test_dir"] = test_dir
    params["spec_file"] = "tests"
    params["unique_test_id"] = "abc123"
    params["min_reported_time"] = 0
    params.addPrivateParam("_validation_classes", [])
    return params


class TestHarnessTester(TestHarnessTestCase):
    def testDefaultKeepsOutputAfterRun(self):
        """
        Test that the default output deletion behavior only deletes before running.
        """

        def post_run(result):
            job = self.getJobWithName(result.harness, "keeps_output")
            output = os.path.join(job.getTester().getTestDir(), "cleanup.txt")
            self.assertTrue(os.path.exists(output))

        self.runTests(
            tests={
                "keeps_output": {
                    "type": "CheckFiles",
                    "command": "/usr/bin/touch",
                    "cli_args": "cleanup.txt",
                    "check_files": "cleanup.txt",
                }
            },
            minimal_capabilities=True,
            post_run=post_run,
        )

    def testFalseDeletesOutputAfterRun(self):
        """
        Test that disabling pre-run deletion deletes declared outputs after running.
        """

        def post_run(result):
            job = self.getJobWithName(result.harness, "deletes_output")
            output = os.path.join(job.getTester().getTestDir(), "cleanup.txt")
            self.assertFalse(os.path.exists(output))

        self.runTests(
            tests={
                "deletes_output": {
                    "type": "CheckFiles",
                    "command": "/usr/bin/touch",
                    "cli_args": "cleanup.txt",
                    "check_files": "cleanup.txt",
                    "delete_output_before_running": False,
                }
            },
            minimal_capabilities=True,
            post_run=post_run,
        )

    def testRecoverPart2KeepsOutputAfterRun(self):
        """
        Test that the harness-generated recover part2 cleanup setting does not delete outputs.
        """

        with tempfile.TemporaryDirectory() as test_dir:
            output = os.path.join(test_dir, "cleanup.txt")
            open(output, "w").close()

            params = _make_check_files_params(test_dir)
            params.setParamByUser("delete_output_before_running", False)
            tester = CheckFiles("recover_keeps_output", params)
            self.assertTrue(tester._delete_output_after_running)

            tester.parameters()["delete_output_before_running"] = False
            tester.setDeleteOutputAfterRunning(False)
            tester.postRun(None)

            self.assertTrue(os.path.exists(output))
