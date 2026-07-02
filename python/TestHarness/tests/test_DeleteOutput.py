# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import os

from TestHarnessTestCase import TestHarnessTestCase


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
