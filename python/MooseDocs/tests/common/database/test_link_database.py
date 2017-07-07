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

import os
import unittest
import MooseDocs
from MooseDocs.common.MooseLinkDatabase import MooseLinkDatabase

class TestMooseLinkDatabase(unittest.TestCase):
    """
    Tests for MooseLinkDatabase object.
    """

    @classmethod
    def setUpClass(cls):
        """
        Create link database.
        """
        config = MooseDocs.load_config(os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'website.yml'))
        options = config['MooseDocs.extensions.app_syntax']
        cls.database = MooseLinkDatabase(repo=options['repo'], links=options['links'])

    def testTests(self):
        """
        Look for class in input files.
        """
        # The BoxMarker object is something that is nested and not usually listed first, so it is a
        # good test case that the regex is getting down into the nested items.
        self.assertIn('BoxMarker', self.database.inputs['Tests'],
                      'BoxMarker not located in database!')

    def testExamples(self):
        """
        The ExampleDiffusion class should be in input files and Kernel inherited from.
        """
        self.assertIn('ExampleDiffusion', self.database.inputs['Examples'],
                      'ExampleDiffusion was not found in example input files!')
        self.assertIn('Kernel', self.database.children['Examples'],
                      'Kernel was not used in the example source code!')

    def testTutorials(self):
        """
        Test the tutorials directory is properly searched.
        """
        self.assertIn('DarcyPressure', self.database.inputs['Tutorials'],
                      'DarcyPressue was not found in tutorial input files!')
        self.assertIn('Diffusion', self.database.children['Tutorials'],
                      'Diffusion was not used in the tutorial source code!')

    def testSource(self):
        """
        Test the tutorials directory is properly searched.
        """
        self.assertIn('Diffusion', self.database.children['Source'],
                      'Diffusion was not found in source code.!')


if __name__ == '__main__':
    unittest.main(verbosity=2)
