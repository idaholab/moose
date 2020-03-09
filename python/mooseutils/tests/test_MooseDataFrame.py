#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import shutil
import unittest
import time
import mooseutils

class TestMooseDataFrame(unittest.TestCase):
    """
    Test use of MooseDataFrame for loading/reloading csv files.
    """

    def setUp(self):
        """
        Define the test filename.
        """
        self._filename = '../../test_files/white_elephant_jan_2016.csv'
        self._keys = ['air_temp_low_24_hour_set_1', 'snow_depth_set_1']

    def testBasic(self):
        """
        Test that if a file exists it is loaded w/o error.
        """

        # Test basic read
        data = mooseutils.MooseDataFrame(self._filename)
        self.assertEqual(self._filename, data.filename)
        self.assertTrue(data)

        # Key Testing
        for k in self._keys:
            self.assertTrue(k in data)

        # Check data
        x = data[self._keys]
        self.assertEqual(x.loc[10][self._keys[0]], 2.12)
        self.assertEqual(x.loc[10][self._keys[1]], 51.00)

    def testNoFile(self):
        """
        Test that no-file doesn't fail.
        """

        filename = 'not_a_file.csv'
        data = mooseutils.MooseDataFrame(filename)
        self.assertEqual(filename, data.filename)
        self.assertFalse(data)

        # Key Testing
        self.assertFalse('key' in data)

        x = data[ ['key1', 'key2'] ]
        self.assertTrue(x.empty)

    def testEmptyUpdateRemove(self):
        """
        Test that data appears when file is loaded.
        """

        # Temporary filename
        filename = "{}_{}.csv".format(self.__class__.__name__, 'tmp')
        if os.path.exists(filename):
            os.remove(filename)

        # (1) No-file
        data = mooseutils.MooseDataFrame(filename)
        self.assertEqual(filename, data.filename)
        for k in self._keys:
            self.assertFalse(k in data)
        x = data[self._keys]
        self.assertTrue(x.empty)

        # (2) Data exists
        shutil.copyfile(self._filename, filename)
        data.update()
        for k in self._keys:
            self.assertTrue(k in data)
        x = data[self._keys]
        self.assertEqual(x.loc[10][self._keys[0]], 2.12)
        self.assertEqual(x.loc[10][self._keys[1]], 51.00)
        self.assertFalse(x.empty)

        # (3) Data remove
        os.remove(filename)
        data.update()
        for k in self._keys:
            self.assertFalse(k in data)
        x = data[self._keys]
        self.assertTrue(x.empty)

    def testIndex(self):
        """
        Test that the index of the data may be set.
        """
        data = mooseutils.MooseDataFrame(self._filename, index='time')
        x = data[self._keys]
        idx = 29.42
        self.assertEqual(x.loc[idx][self._keys[0]], 20.12)
        self.assertEqual(x.loc[idx][self._keys[1]], 59.00)

    def testOldFile(self):
        """
        Test that "old" files do not load.
        """
        data = mooseutils.MooseDataFrame(self._filename, index='time')
        self.assertTrue(data)
        data = mooseutils.MooseDataFrame(self._filename, index='time', run_start_time=time.time())
        self.assertFalse(data)


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
