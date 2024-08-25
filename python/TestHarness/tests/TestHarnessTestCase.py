#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import unittest
import subprocess
import tempfile
import re

class TestHarnessTestCase(unittest.TestCase):
    """
    TestCase class for running TestHarness commands.
    """

    def runTests(self, *args, tmp_output=True):
        cmd = ['./run_tests'] + list(args) + ['--term-format', 'njCst']
        sp_kwargs = {'cwd': os.path.join(os.getenv('MOOSE_DIR'), 'test'),
                     'text': True}
        if tmp_output:
            with tempfile.TemporaryDirectory() as output_dir:
                cmd += ['-o', output_dir]
            return subprocess.check_output(cmd, **sp_kwargs)
        return subprocess.check_output(cmd, **sp_kwargs)

    def runExceptionTests(self, *args, tmp_output=True):
        try:
            self.runTests(*args, tmp_output=tmp_output)
        except Exception as err:
            return err.output
        raise RuntimeError('test failed to fail')

    def checkStatus(self, output, passed=0, skipped=0, pending=0, failed=0):
        """
        Make sure the TestHarness status line reports the correct counts.
        """
        # We need to be sure to match any of the terminal codes in the line
        status_re = r'(?P<passed>\d+) passed.*, .*(?P<skipped>\d+) skipped.*, .*(?P<failed>\d+) failed'
        match = re.search(status_re, output, re.IGNORECASE)
        self.assertNotEqual(match, None)
        self.assertEqual(match.group("passed"), str(passed))
        self.assertEqual(match.group("failed"), str(failed))
        self.assertEqual(match.group("skipped"), str(skipped))
