#!/usr/bin/env python
#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
import unittest
from MooseDocs.testing import MarkdownTestCase

class TestImage(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.media', 'MooseDocs.extensions.refs']

    def testDefault(self):
        md = '!media docs/media/github-logo.png'
        self.assertConvert('testDefault.html', md)

    def testDefaultId(self):
        md = '!media docs/media/github-logo.png id=github'
        self.assertConvert('testDefaultId.html', md)

    def testDisableCount(self):
        md = '!media docs/media/github-logo.png\n\n'
        md += '!media docs/media/github-logo.png id=github'
        self.assertConvert('testDisableCount.html', md)

    def testCount(self):
        md = '!media docs/media/github-logo.png id=github1\n\n'
        md += '!media docs/media/inl_blue.png id=github2'
        self.assertConvert('testCount.html', md)

    def testChangeCounter(self):
        md = '!media docs/media/github-logo.png id=fig1\n\n'
        md += '!media docs/media/inl_blue.png counter=foo id=foo1\n\n'
        md += '!media docs/media/github-logo.png id=fig2\n\n'
        self.assertConvert('testChangeCounter.html', md)

    def testDisableMaterializeBox(self):
        md = '!media docs/media/github-logo.png materialboxed=false'
        self.assertConvert('testDisableMaterializeBox.html', md)

    def testCaption(self):
        md = '!media docs/media/github-logo.png caption=A test caption'
        self.assertConvert('testCaption.html', md)

    def testSettings(self):
        md = '!media docs/media/github-logo.png float=right width=30%'
        self.assertConvert('testSettings.html', md)

    def testCard(self):
        md = '!media docs/media/github-logo.png card=true'
        self.assertConvert('testCard.html', md)

    def testCardCaption(self):
        md = '!media docs/media/github-logo.png card=1 caption=A test caption'
        self.assertConvert('testCardCaption.html', md)

    def testFileError(self):
        md = '!media docs/media/not_a_file.png'
        self.assertConvert('testFileError.html', md)

    def testRef(self):
        md = '!media docs/media/github-logo.png id=foo\n\n\\ref{foo}'
        self.assertConvert('testRef.html', md)

if __name__ == '__main__':
    unittest.main(verbosity=2)
