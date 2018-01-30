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

import re
import unittest
import MooseDocs
from MooseDocs.testing import MarkdownTestCase

class TestDevelExtension(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.devel']

    def testBuildStatus(self):
        md = '!buildstatus https://moosebuild.org/mooseframework/ float=right padding-left=10px'
        self.assertConvert('test_BuildStatus.html', md)

    def testPackage(self):
        md = '!moosepackage arch=centos7 return=link!'
        html = self.convert(md)
        self.assertIn(r'<a href="http://mooseframework.org/static/media/uploads/files', html)

    def testConfig(self):
        md = '!extension DevelExtension'
        self.assertConvert('test_Config.html', md)

    def testSettings(self):
        md = '!extension-settings moose_extension_config'
        self.assertConvert('test_Settings.html', md)

    def testDeprecated(self):
        MooseDocs.DEPRECATED_MARKDOWN = [(re.compile(r'(?P<command>!testing!)'), '!replace!')]
        md = 'This is a test\nof deprecated commands !testing!'
        self.convert(md)
        self.assertInLogError('!testing!')

if __name__ == '__main__':
    unittest.main(verbosity=2)
