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

class TestRef(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.refs', 'MooseDocs.extensions.listings']
    def testRef(self):
        md = '!listing id=foo\n```testing```\n\n\\ref{foo}'
        self.assertConvert('testRef.html', md)

    def testUnknownRef(self):
        md = '\\ref{unknown}'
        self.assertConvert('testUnknownRef.html', md)

if __name__ == '__main__':
    unittest.main(verbosity=2)
