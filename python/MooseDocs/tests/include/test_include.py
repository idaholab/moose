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
import MooseDocs
from MooseDocs.testing import MarkdownTestCase

class TestInclude(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.include']
    ONE = os.path.join(MooseDocs.ROOT_DIR, 'python', 'MooseDocs', 'tests', 'input', 'one.md')
    TWO = ONE.replace('one.md', 'two.md')
    THREE = ONE.replace('one.md', 'three.md')

    def testInclude(self):
        md = '!include {}'.format(os.path.join('python', 'MooseDocs', 'tests', 'include', 'one.md'))
        html = self.convert(md)
        self.assertIn('Congress shall make no law', html)
        self.assertIn('A well regulated Militia', html)
        self.assertIn('No Soldier shall', html)
        self.assertNotIn('The right of the people', html)
        self.assertNotIn('!include', html)

if __name__ == '__main__':
    unittest.main(verbosity=2)
