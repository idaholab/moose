#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
import unittest
import mooseutils

ROOT_DIR = mooseutils.git_root_dir()
sys.path.insert(0, os.path.join(ROOT_DIR, 'scripts'))
from mooseutils import check_requirement

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
