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

class TestImage(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.media', 'MooseDocs.extensions.refs']

    def testDefault(self):
        md = '!media media/github-logo.png'
        self.assertConvert('testDefault.html', md)

    def testDefaultId(self):
        md = '!media media/github-logo.png id=github'
        self.assertConvert('testDefaultId.html', md)

    def testDisableCount(self):
        md = '!media media/github-logo.png\n\n'
        md += '!media media/github-logo.png id=github'
        self.assertConvert('testDisableCount.html', md)

    def testCount(self):
        md = '!media media/github-logo.png id=github1\n\n'
        md += '!media media/inl_blue.png id=github2'
        self.assertConvert('testCount.html', md)

    def testChangeCounter(self):
        md = '!media media/github-logo.png id=fig1\n\n'
        md += '!media media/inl_blue.png counter=foo id=foo1\n\n'
        md += '!media media/github-logo.png id=fig2\n\n'
        self.assertConvert('testChangeCounter.html', md)

    def testDisableMaterializeBox(self):
        md = '!media media/github-logo.png materialboxed=false'
        self.assertConvert('testDisableMaterializeBox.html', md)

    def testCaption(self):
        md = '!media media/github-logo.png caption=A test caption'
        self.assertConvert('testCaption.html', md)

    def testSettings(self):
        md = '!media media/github-logo.png float=right width=30%'
        self.assertConvert('testSettings.html', md)

    def testCard(self):
        md = '!media media/github-logo.png card=true'
        self.assertConvert('testCard.html', md)

    def testCardCaption(self):
        md = '!media media/github-logo.png card=1 caption=A test caption'
        self.assertConvert('testCardCaption.html', md)

    def testRef(self):
        md = '!media media/github-logo.png id=foo\n\n\\ref{foo}'
        self.assertConvert('testRef.html', md)

if __name__ == '__main__':
    unittest.main(verbosity=2)
