#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, io
import unittest
import mock
import TestHarness
from contextlib import redirect_stdout

class TestHarnessTester(unittest.TestCase):
    @mock.patch.object(TestHarness.util, 'getExeJSON')
    def mocked_output(self, mocked, expect_fail, mocked_return):
        MOOSE_DIR = os.getenv('MOOSE_DIR')
        os.chdir(f'{MOOSE_DIR}/test')
        out = io.StringIO()
        with redirect_stdout(out):
            mocked_return.return_value=f'{mocked}'
            with self.assertRaises(SystemExit) as e:
                args = ['', '-i', 'required_objects']
                TestHarness.TestHarness.buildAndRun(args, None, MOOSE_DIR)
            if expect_fail:
                self.assertNotEqual(e.exception.code, 0)
            else:
                self.assertEqual(e.exception.code, 0)
        return out.getvalue()

    def testGoodJSONOutput(self):
        """
        Test for good json output
        """
        out = self.mocked_output('**START JSON DATA**\n{}**END JSON DATA**\n', False)
        self.assertIn('AnalyticalIndicator not found in executable', out)

    def testBadJSONOutput(self):
        """
        Test for bad json output
        """
        with self.assertRaises(Exception) as e:
            self.mocked_output('**START JSON DATA**\n{badjson}**END JSON DATA**\n', True)
        self.assertEqual(str(e.exception), 'Failed to parse JSON from --json')

    def testBadIndex(self):
        """
        Test for general bad output (unable to split)
        """
        with self.assertRaises(Exception) as e:
            self.mocked_output('bad output\n', True)
        self.assertEqual(str(e.exception), 'Failed to find JSON header and footer from --json')
