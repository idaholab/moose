#!/usr/bin/env python
import os
import unittest
from MooseDocs.testing import MarkdownTestCase

class TestMooseImageFile(MarkdownTestCase):
    """
    Test commands in MooseTextFile extension.
    """

    def testImage(self):
        md = '!image docs/media/github-logo.png'
        self.assertConvert('test_Image.html', md)

    def testImageCaption(self):
        md = '!image docs/media/github-logo.png caption=A test caption'
        self.assertConvert('test_ImageCaption.html', md)

    def testImageSettings(self):
        md = '!image docs/media/github-logo.png float=right width=30%'
        self.assertConvert('test_ImageSettings.html', md)

    def testImageBadFile(self):
        md = '!image docs/media/not_a_file.png'
        self.assertConvert('test_ImageBadFile.html', md)

if __name__ == '__main__':
    unittest.main(verbosity=2)
