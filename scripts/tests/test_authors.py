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

class Test(unittest.TestCase):

    def testLocationFolders(self):
        locations = [os.path.join(mooseutils.git_root_dir(), 'python', 'mooseutils'),
                     os.path.join(mooseutils.git_root_dir(), 'python', 'moosesqa')]

        out = mooseutils.check_output(['./authors.py', *locations, '-j', '1'],
                                      cwd=os.path.join(mooseutils.git_root_dir(), 'scripts'))
        self.assertIn('Andrew', out)
        self.assertIn('C++', out)
        self.assertIn('Python', out)
        self.assertIn('Input', out)
        self.assertIn('Markdown', out)
        self.assertIn('Make', out)
        self.assertIn('YAML', out)
        self.assertIn('Total', out)
        self.assertIn('TOTAL', out)

    def testLanguage(self):
        locations = [os.path.join(mooseutils.git_root_dir(), 'python', 'mooseutils')]
        out = mooseutils.check_output(['./authors.py', *locations, '-j', '1', '-l', 'Python'],
                                      cwd=os.path.join(mooseutils.git_root_dir(), 'scripts'))
        self.assertIn('Andrew', out)
        self.assertNotIn('C++', out)
        self.assertIn('Python', out)
        self.assertNotIn('Input', out)
        self.assertNotIn('Markdown', out)
        self.assertNotIn('Make', out)
        self.assertNotIn('YAML', out)
        self.assertIn('Total', out)
        self.assertIn('TOTAL', out)


if __name__ == '__main__':
    unittest.main(verbosity=2, buffer=True)
