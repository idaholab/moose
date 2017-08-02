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
#pylint: enable=missing-docstring
import unittest
from MooseDocs.common import slugify

class TestSlugify(unittest.TestCase):

    INVALID = [':', '\\', '.', '[', ']', '(', ')', '!', '"', "'", '*', '?', '<', '>', '|']

    def testBasic(self):
        """
        Test basic conversion and invalid characters.
        """
        self.assertEqual(('Foo_Bar', True), slugify('Foo Bar'))
        for x in self.INVALID:
            string = 'Foo{}Bar'.format(x)
            self.assertEqual((string, False), slugify(string))

    def testReplace(self):
        """
        Test replacement.
        """
        for x in self.INVALID:
            string = 'Foo{}Bar'.format(x)
            replaced = 'FooXBar'.format(x)
            self.assertEqual((replaced, True), slugify(string, (x, 'X')))


if __name__ == '__main__':
    unittest.main(verbosity=2)
