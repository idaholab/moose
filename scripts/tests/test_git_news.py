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
from git_news import main

class Test(unittest.TestCase):

    def testScript(self):
        """Test the get_news.py script"""
        proc = main()
        self.assertFalse(proc.returncode)

if __name__ == '__main__':
    unittest.main(verbosity=2, buffer=True)
