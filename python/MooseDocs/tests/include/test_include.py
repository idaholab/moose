#!/usr/bin/env python
#pylint: disable=missing-docstring
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
import MooseDocs
from MooseDocs.testing import MarkdownTestCase

class TestInclude(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.include']
    ONE = os.path.join(MooseDocs.ROOT_DIR, 'python', 'MooseDocs', 'tests', 'input', 'one.md')
    TWO = ONE.replace('one.md', 'two.md')
    THREE = ONE.replace('one.md', 'three.md')

    def testInclude(self):
        md = '!include {}'.format(os.path.join('python', 'MooseDocs', 'tests', 'include', 'one.md'))
        html = self.convert(md)
        self.assertIn('Congress shall make no law', html)
        self.assertIn('A well regulated Militia', html)
        self.assertIn('No Soldier shall', html)
        self.assertNotIn('The right of the people', html)
        self.assertNotIn('!include', html)

if __name__ == '__main__':
    unittest.main(verbosity=2)
