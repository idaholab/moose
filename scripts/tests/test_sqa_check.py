#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import sys
import subprocess
import unittest

@unittest.skipIf(sys.version_info < (3, 7), "Requires Python 3.7 or greater.")
class Test(unittest.TestCase):

    def testScript(self):
        """Test that the SQA checker script runs"""
        proc = subprocess.run(['./sqa_check.py', '--config', 'tests/test_sqa.hit'], capture_output=True, check=False, cwd='..', encoding='utf8')
        self.assertIn('Collecting SQA document reports...', proc.stdout)
        self.assertIn('Collecting SQA requirement reports...', proc.stdout)
        self.assertIn('Collecting SQA application reports...', proc.stdout)

    def testReport(self):
        proc = subprocess.run(['./sqa_check.py', '--config', 'tests/test_sqa.hit', '--reports', 'doc'], capture_output=True, check=False, cwd='..', encoding='utf8')
        self.assertIn('Collecting SQA document reports...', proc.stdout)
        self.assertNotIn('Collecting SQA requirement reports...', proc.stdout)
        self.assertNotIn('Collecting SQA application reports...', proc.stdout)

        proc = subprocess.run(['./sqa_check.py', '--config', 'tests/test_sqa.hit', '--reports', 'doc', 'req'], capture_output=True, check=False, cwd='..', encoding='utf8')
        self.assertIn('Collecting SQA document reports...', proc.stdout)
        self.assertIn('Collecting SQA requirement reports...', proc.stdout)
        self.assertNotIn('Collecting SQA application reports...', proc.stdout)

        proc = subprocess.run(['./sqa_check.py', '--config', 'tests/test_sqa.hit', '--reports', 'doc', 'req', 'app'], capture_output=True, check=False, cwd='..', encoding='utf8')
        self.assertIn('Collecting SQA document reports...', proc.stdout)
        self.assertIn('Collecting SQA requirement reports...', proc.stdout)
        self.assertIn('Collecting SQA application reports...', proc.stdout)

if __name__ == '__main__':
    unittest.main(verbosity=2, buffer=True)
