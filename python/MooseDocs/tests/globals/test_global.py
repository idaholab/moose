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

class TestGlobalExtension(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.global']

    @classmethod
    def updateExtensions(cls, configs):
        """
        Add a import filename.
        """
        configs['MooseDocs.extensions.global']['import'] = \
            ['python/MooseDocs/tests/globals/test_import.yml']

    def testGlobalConfig(self):
        md = '[libMesh]'
        html = self.convert(md)
        self.assertIn('<a href="http://libmesh.github.io/">libMesh</a>', html)

    def testImportConfig(self):
        md = '[Google]'
        html = self.convert(md)
        self.assertIn('<a href="http://www.google.com">Google</a>', html)

if __name__ == '__main__':
    unittest.main(verbosity=2)
