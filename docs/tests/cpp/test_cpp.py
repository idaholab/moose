#!/usr/bin/env python
import os
import unittest
import MooseDocs
from MooseDocs.testing import MarkdownTestCase

class TestMooseMooseCppMethod(MarkdownTestCase):
    """
    Test commands in MooseCppMethod
    """

    def testClang(self):
        md = '!clang framework/src/kernels/Diffusion.C method=computeQpResidual'
        self.assertConvert('test_Clang.html', md)

    def testClangBadFile(self):
        md = '!clang framework/src/kernels/NotARealFile.C method=computeQpResidual'
        self.assertConvert('test_ClangBadFile.html', md)

    def testClangNoMethod(self):
        md = '!clang framework/src/kernels/Diffusion.C'
        self.assertConvert('test_ClangNoMethod.html', md)

    def testClangBadMethod(self):
        md = '!clang framework/src/kernels/Diffusion.C method=not_a_valid_function'
        self.assertConvert('test_ClangBadMethod.html', md)

if __name__ == '__main__':
    unittest.main(verbosity=2)
