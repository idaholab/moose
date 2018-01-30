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

import unittest
from MooseDocs.testing import MarkdownTestCase

class TestSlider(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.media']
    def testDefault(self):
        self.assertConvertFile('testDefault.html', 'testDefault.md')

    def testDefaultId(self):
        self.assertConvertFile('testDefaultId.html', 'testDefaultId.md')

if __name__ == '__main__':
    unittest.main(verbosity=2)
