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
from MooseDocs.testing import MarkdownTestCase

class TestCSS(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.css']
    def testCSS(self):
        self.assertConvertFile('test_css.html', os.path.join(self.WORKING_DIR, 'css.md'))

    def testCSSList(self):
        self.assertConvertFile('test_css_list.html', os.path.join(self.WORKING_DIR, 'css_list.md'))

if __name__ == '__main__':
    unittest.main(verbosity=2)
