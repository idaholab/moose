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

import os
import unittest
from MooseDocs.testing import MarkdownTestCase

class TestCSS(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.css']
    def testCSS(self):
        self.assertConvertFile('test_css.html', os.path.join(self.WORKING_DIR, 'css.md'))

    def testCSSList(self):
        self.assertConvertFile('test_css_list.html', os.path.join(self.WORKING_DIR, 'css_list.md'))

if __name__ == '__main__':
    unittest.main(verbosity=2)
