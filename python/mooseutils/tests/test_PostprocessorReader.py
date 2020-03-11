#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import os
import shutil
import unittest
import subprocess
import time
import mooseutils

class TestrPostprocessorReader(unittest.TestCase):
    """
    Test use of PostprocessorReader for loading/reloading csv files.

    The PostprocessorReader is an extension of MooseDataFrame, so only the new functions are tested here.
    """

    def setUp(self):
        """
        Define the test filename.
        """
        self._partial = os.path.abspath('../../test_files/white_elephant_jan_2016_partial.csv')
        self._filename = os.path.abspath('../../test_files/white_elephant_jan_2016.csv')
        self._keys = ['air_temp_low_24_hour_set_1', 'snow_depth_set_1']

    def testBasic(self):
        """
        Test that if a file exists it is loaded w/o error.
        """

        # Test basic read
        data = mooseutils.PostprocessorReader(self._filename)
        self.assertEqual(self._filename, data.filename)
        self.assertTrue(data)

        # Key Testing
        for k in self._keys:
            self.assertTrue(k in data)

        # Check data
        x = data[self._keys]
        self.assertEqual(x.loc[10][self._keys[0]], 2.12)
        self.assertEqual(x.loc[10][self._keys[1]], 51.00)

    def testCall(self):
        """
        Test that operator() method is working.
        """
        data = mooseutils.PostprocessorReader(self._filename)

        # Single key
        x = data[self._keys[0]]
        self.assertEqual(x.loc[10], 2.12)

        # Multiple keys
        x = data[self._keys]
        self.assertEqual(x.loc[10][self._keys[0]], 2.12)
        self.assertEqual(x.loc[10][self._keys[1]], 51.00)

    def testNewDataReload(self):
        """
        Test that new data is loaded automatically.
        """

        # Copy partial data
        tmp = "{}.csv".format(self.__class__.__name__)
        shutil.copyfile(self._partial, tmp)

        # Load data and inspect
        data = mooseutils.PostprocessorReader(tmp)
        self.assertEqual(data.data.shape, (287,8))

        # Wait and copy more data
        time.sleep(1)
        shutil.copyfile(self._filename, tmp)
        data.update()
        self.assertEqual(data.data.shape, (742,8))
        os.remove(tmp)

    def testVariables(self):
        """
        Test the the variables names are being read.
        """
        data = mooseutils.PostprocessorReader(self._filename)
        self.assertTrue(data)
        self.assertIn('time', data.variables())
        for k in self._keys:
            self.assertIn(k, data.variables())

    def testRepr(self):
        """
        Test the 'repr' method for writing scripts is working.
        """

        # Load the files
        data = mooseutils.PostprocessorReader(self._filename)
        self.assertTrue(data)

        # Get script text
        output, imports = data.repr()

        # Append testing content
        output += ["print('SHAPE:', data.data.shape)"]
        output += ["print('VALUE:', data['snow_depth_set_1'][10])"]

        # Write the test script
        script = '{}_repr.py'.format(self.__class__.__name__)
        with open(script, 'w') as fid:
            fid.write('\n'.join(imports))
            fid.write('\n'.join(output))

        # Run script
        self.assertTrue(os.path.exists(script))
        out = subprocess.check_output(['python', script])

        # Test for output
        self.assertIn('SHAPE: (742, 8)', out.decode())
        self.assertIn('VALUE: 51', out.decode())

        # Remove the script
        os.remove(script)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True)
