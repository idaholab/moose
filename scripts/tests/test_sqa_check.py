#!/usr/bin/env python
import os
import sys
import unittest
import mooseutils

ROOT_DIR = mooseutils.git_root_dir()
sys.path.insert(0, os.path.join(ROOT_DIR, 'scripts'))
from sqa_check import check_requirement

class Test(unittest.TestCase):
    def testScript(self):
        """Test the SQA checker."""
        check_requirement('[Tests][foo][][]')
        output = sys.stdout.getvalue()
        self.assertIn('requirement', output)
        self.assertIn('design', output)
        self.assertIn('issues', output)

if __name__ == '__main__':
    unittest.main(verbosity=2, buffer=True)
