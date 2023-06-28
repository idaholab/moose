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
import re

class TestHarnessTestCase(unittest.TestCase):
    """
    TestCase class for running TestHarness commands.
    """

    def runExceptionTests(self, *args):
        os.environ['MOOSE_TERM_FORMAT'] = 'njCst'
        cmd = ['./run_tests'] + list(args)
        try:
            return subprocess.check_output(cmd, cwd=os.path.join(os.getenv('MOOSE_DIR'), 'test'))
            raise RuntimeError('test failed to fail')
        except Exception as err:
            return err.output

    def runTests(self, *args):
        os.environ['MOOSE_TERM_FORMAT'] = 'njCst'
        cmd = ['./run_tests'] + list(args)
        return subprocess.check_output(cmd, cwd=os.path.join(os.getenv('MOOSE_DIR'), 'test'))

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
