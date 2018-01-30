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

class TestMooseMooseCppMethod(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.listings']

    def testClang(self):
        md = '!listing framework/src/kernels/Diffusion.C method=computeQpResidual'
        self.assertConvert('testClang.html', md)

    def testClangDeclaration(self):
        md = '!listing framework/src/kernels/Diffusion.C method=computeQpResidual declaration=1'
        self.assertConvert('testClangDeclaration.html', md)

    def testClangMethodError(self):
        md = '!listing framework/src/kernels/Diffusion.C method=not_a_valid_function'
        self.assertConvert('testClangMethodError.html', md)

if __name__ == '__main__':
    unittest.main(verbosity=2)
