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
    def testLink(self):
        self.assertConvert('test_Link.html', '[libMesh]')

if __name__ == '__main__':
    unittest.main(verbosity=2)
