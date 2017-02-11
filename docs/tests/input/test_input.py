#!/usr/bin/env python
import os
import unittest
from MooseDocs.testing import MarkdownTestCase

class TestMooseInputBlock(MarkdownTestCase):
    """
    Test commands in MooseObjectSyntax extension.
    """

    def testInput(self):
        md = '!input test/tests/kernels/simple_diffusion/simple_diffusion.i block=Kernels'
        self.assertConvert('test_Input.html', md)

    def testInputNoBlock(self):
        md = '!input test/tests/kernels/simple_diffusion/simple_diffusion.i'
        self.assertConvert('test_InputNoBlock.html', md)

    def testInputBadBlock(self):
        md = '!input test/tests/kernels/simple_diffusion/simple_diffusion.i block=BadBlock'
        self.assertConvert('test_InputBadBlock.html', md)

    def testInputError(self):
        md = '!input this/is/not/a/valid/file.i'
        self.assertConvert('test_InputError.html', md)

if __name__ == '__main__':
    unittest.main(verbosity=2)
