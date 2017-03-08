#!/usr/bin/env python
#pylint: disable=missing-docstring
#################################################################
#                   DO NOT MODIFY THIS HEADER                   #
#  MOOSE - Multiphysics Object Oriented Simulation Environment  #
#                                                               #
#            (c) 2010 Battelle Energy Alliance, LLC             #
#                      ALL RIGHTS RESERVED                      #
#                                                               #
#           Prepared by Battelle Energy Alliance, LLC           #
#             Under Contract No. DE-AC07-05ID14517              #
#              With the U. S. Department of Energy              #
#                                                               #
#              See COPYRIGHT for full restrictions              #
#################################################################
import os
import unittest
import time
import mooseutils
import chigger

class Test_getActiveFilenames(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """
        Copy the temporary files to working directory.
        """
        cls.basename = cls.__name__
        cls.testfiles = chigger.utils.copy_adaptive_exodus_test_files(cls.basename)

    @classmethod
    def tearDownClass(cls):
        """
        Cleanup test files
        """
        for fname in cls.testfiles:
            if os.path.exists(fname): os.remove(fname)

    def testBasic(self):
        """
        Test that all files can be read.
        """
        active = chigger.utils.get_active_filenames(self.basename + '.e', self.basename + '.e-s*')
        self.assertEqual(len(active), 9)
        self.assertEqual(active[0][0], self.basename + '.e')
        self.assertEqual(active[-1][0], self.basename + '.e-s009')

    def testUpdate(self):
        """
        Test that updating the files updates the active list.
        """

        # Wait and the "update" the first few files
        time.sleep(1.5)
        for i in range(5):
            print self.testfiles[i]
            mooseutils.touch(self.testfiles[i])

        active = chigger.utils.get_active_filenames(self.basename + '.e', self.basename + '.e-s*')
        self.assertEqual(len(active), 5)
        self.assertEqual(active[0][0], self.basename + '.e')
        self.assertEqual(active[-1][0], self.basename + '.e-s005')

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
