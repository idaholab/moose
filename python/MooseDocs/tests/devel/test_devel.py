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

if __name__ == '__main__':
    unittest.main(verbosity=2)
