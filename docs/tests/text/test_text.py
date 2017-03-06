#!/usr/bin/env python
import os
import unittest
from MooseDocs.testing import MarkdownTestCase

class TestMooseTextFile(MarkdownTestCase):
    """
    Test commands in MooseTextFile extension.
    """

    def testText(self):
        md = '!text test/tests/kernels/simple_diffusion/simple_diffusion.i'
        self.assertConvert('test_Text.html', md)

    def testTextStart(self):
        md = '!text test/tests/kernels/simple_diffusion/simple_diffusion.i start=[Outputs]'
        self.assertConvert('test_TextStart.html', md)

    def testTextEnd(self):
        md = '!text test/tests/kernels/simple_diffusion/simple_diffusion.i end=[Variables]'
        self.assertConvert('test_TextEnd.html', md)

    def testTextStartEnd(self):
        md = '!text test/tests/kernels/simple_diffusion/simple_diffusion.i start=[Variables] end=[BCs]'
        self.assertConvert('test_TextStartEnd.html', md)

    def testTextStartEndIncludeEnd(self):
        md = '!text test/tests/kernels/simple_diffusion/simple_diffusion.i start=[Variables] end=[BCs] include_end=True'
        self.assertConvert('test_TextStartEndIncludeEnd.html', md)

    def testTextBadFile(self):
        md = '!text test/tests/kernels/simple_diffusion/not_a_file.'
        self.assertConvert('test_TextBadFile.html', md)

if __name__ == '__main__':
    unittest.main(verbosity=2)
