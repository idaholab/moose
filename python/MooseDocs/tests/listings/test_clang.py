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

class TestMooseMooseCppMethod(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.listings']

    def testClang(self):
        md = '!listing framework/src/kernels/Diffusion.C method=computeQpResidual'
        self.assertConvert('testClang.html', md)

    def testClangDeclaration(self):
        md = '!listing framework/src/kernels/Diffusion.C method=computeQpResidual declaration=1'
        self.assertConvert('testClangDeclaration.html', md)

    def testClangMethodError(self):
        md = '!listing framework/src/kernels/Diffusion.C method=not_a_valid_function'
        self.assertConvert('testClangMethodError.html', md)

if __name__ == '__main__':
    unittest.main(verbosity=2)
