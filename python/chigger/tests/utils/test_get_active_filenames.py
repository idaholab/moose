#!/usr/bin/env python3
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

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
            print(self.testfiles[i])
            mooseutils.touch(self.testfiles[i])

        active = chigger.utils.get_active_filenames(self.basename + '.e', self.basename + '.e-s*')
        self.assertEqual(len(active), 5)
        self.assertEqual(active[0][0], self.basename + '.e')
        self.assertEqual(active[-1][0], self.basename + '.e-s005')

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
