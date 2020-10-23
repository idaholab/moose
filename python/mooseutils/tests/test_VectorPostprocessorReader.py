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
import glob
import shutil
import unittest
import mooseutils
import time
import subprocess

class TestVectorPostprocessorReader(unittest.TestCase):
    """
    Test use of MooseDataFrame for loading/reloading csv files.
    """

    def setUp(self):
        """
        Define the test filename.
        """
        self._pattern = os.path.abspath('../../test_files/vpp_*.csv')

    def copyfiles(self, copytime=True):
        """
        Copy the saved file locally.
        """
        for f in sorted(glob.glob(self._pattern)):
            if f.endswith('time.csv') and not copytime:
                continue
            shutil.copyfile(f, os.path.basename(f))

    def tearDown(self):
        """
        Remove local copy of files.
        """
        for f in glob.glob(self._pattern):
            fname = os.path.basename(f)
            if os.path.exists(fname):
                os.remove(fname)

    def testBasic(self):
        """
        Test that if a file exists it is loaded w/o error.
        """
        self.copyfiles()

        # Load the data
        data = mooseutils.VectorPostprocessorReader('vpp_*.csv')
        self.assertEqual(data._pattern, os.path.basename(self._pattern))
        self.assertEqual(data.filename, 'vpp_005.csv')
        self.assertTrue(data._timedata)
        self.assertTrue(data)

        # Check axis organized correctly
        self.assertEqual(data.data.data.shape, (7,3))
        self.assertEqual(data.times(), [1,3,7,13])

        # Check data
        y = data['y']
        self.assertEqual(y[6], 4096)

    def testBasicNoTime(self):
        """
        Test that if a file is loaded w/o error (when no time).
        """
        self.copyfiles(copytime=False)

        data = mooseutils.VectorPostprocessorReader('vpp_*.csv')
        self.assertEqual(data._pattern, os.path.basename(self._pattern))
        self.assertEqual(data.filename, 'vpp_005.csv')
        self.assertFalse(data._timedata)
        self.assertTrue(data)

        # Check axis organized correctly
        self.assertEqual(data.data.data.shape, (7,3))

        # Check that times are loaded
        self.assertEqual(data.times(), [0,1,2,5])

        # Check data
        y = data['y']
        self.assertEqual(y[6], 4096)

    def testEmptyUpdateRemove(self):
        """
        Test that non-exist file can be supplied, loaded, and removed.
        """

        # Create object w/o data
        data = mooseutils.VectorPostprocessorReader('vpp_*.csv')
        self.assertEqual(data._pattern, os.path.basename(self._pattern))
        self.assertEqual(data.filename, None)
        self.assertFalse(data._timedata)
        self.assertFalse(data)

        # Update data
        self.copyfiles()
        data.update()
        self.assertTrue(data._timedata)
        self.assertTrue(data)

        # Check axis organized correctly
        self.assertEqual(data.data.data.shape, (7,3))
        self.assertEqual(list(data.times()), [1,3,7,13])
        y = data['y']
        self.assertEqual(y[6], 4096)

        # Remove data
        self.tearDown()
        data.update()
        self.assertFalse(data._timedata)
        self.assertFalse(data)

    def testOldData(self):
        """
        Test that old data is not loaded
        """

        # Load the files
        self.copyfiles()
        data = mooseutils.VectorPostprocessorReader('vpp_*.csv')
        self.assertTrue(data)
        self.assertEqual(data.data.data.shape, (7,3))
        self.assertEqual(data.times(), [1,3,7,13])

        # Make the last file old
        time.sleep(2) # wait so new files have newer modified times
        mooseutils.touch('vpp_000.csv')
        mooseutils.touch('vpp_001.csv')

        # Update and make certain data structure is smaller
        data.update()
        self.assertTrue(data)
        self.assertEqual(data.times(), [1,3])
        self.assertEqual(data.data.data.shape, (6,3))
        self.assertEqual(data.filename, 'vpp_001.csv')

        # Test data
        y = data['y']
        self.assertEqual(y[3], 6)

        # Touch 3 so that, it should show up then
        time.sleep(1)
        mooseutils.touch('vpp_002.csv')
        data.update()
        self.assertTrue(data)
        self.assertEqual(data.filename, 'vpp_002.csv')
        self.assertEqual(data.data.data.shape, (6,3))

        y = data['y']
        self.assertEqual(y[3], 9)

    def testRemoveData(self):
        """
        Test that removing a file is handled correctly.
        """

        # Load the files
        self.copyfiles()
        data = mooseutils.VectorPostprocessorReader('vpp_*.csv')
        self.assertTrue(data)
        self.assertEqual(data.filename, 'vpp_005.csv')
        self.assertEqual(data.data.data.shape, (7,3))
        self.assertEqual(data.times(), [1,3,7,13])

        # Remove the middle file
        os.remove('vpp_001.csv')

        # Update and check results
        data.update()
        self.assertTrue(data)
        self.assertEqual(data.filename, 'vpp_005.csv')
        self.assertEqual(data.data.data.shape, (7,3))
        self.assertEqual(data.times(), [1,7,13])

    def testTimeAccess(self):
        """
        Test that time based data access is working.
        """
        self.copyfiles()
        data = mooseutils.VectorPostprocessorReader('vpp_*.csv')
        self.assertEqual(data.times(), [1,3,7,13])
        self.assertTrue(data)

        # Test that newest data is returned
        self.assertEqual(data.filename, 'vpp_005.csv')
        y = data['y']
        self.assertEqual(y[6], 4096)

        # Test that older data can be loaded
        data.update(time=3)
        self.assertEqual(data.filename, 'vpp_001.csv')
        y = data['y']
        self.assertEqual(y[5], 10)

        # Test that bisect returns value even if time is exactly correct
        data.update(time=7.3)
        self.assertEqual(data.filename, 'vpp_002.csv')
        y = data['y']
        self.assertEqual(y[5], 25)

        # Test that beyond end returns newest
        data.update(time=9999)
        self.assertEqual(data.filename, 'vpp_005.csv')
        y = data['y']
        self.assertEqual(y[6], 4096)

        # Test time less than beginning returns first
        data.update(time=0.5)
        self.assertEqual(data.filename, 'vpp_000.csv')
        y = data['y']
        self.assertEqual(y[5], 5)

    def testVariables(self):
        """
        Check variable names.
        """
        self.copyfiles()
        data = mooseutils.VectorPostprocessorReader('vpp_*.csv')
        self.assertTrue(data)
        self.assertEqual(data.variables(), ['index (Peacock)', 'x', 'y'])

    def testRepr(self):
        """
        Test the 'repr' method for writing scripts is working.
        """

        # Load the files
        self.copyfiles()
        data = mooseutils.VectorPostprocessorReader('vpp_*.csv')
        self.assertTrue(data)

        # Get script text
        output, imports = data.repr()

        # Append testing content
        output += ["print(data['y'][6])"]

        # Write the test script
        script = '{}_repr.py'.format(self.__class__.__name__)
        with open(script, 'w') as fid:
            fid.write('\n'.join(imports))
            fid.write('\n'.join(output))

        # Run script
        self.assertTrue(os.path.exists(script))
        out = mooseutils.check_output(['python', script])

        # Test for output
        self.assertIn('4096', out)

        # Remove the script
        os.remove(script)

    def testUpdateCall(self):
        """
        Test that MooseDataFrame is cached
        """
        self.copyfiles()
        data = mooseutils.VectorPostprocessorReader('vpp_*.csv')
        ids0 = [id(f) for f in data._frames.values()]
        data.update()
        ids1 = [id(f) for f in data._frames.values()]
        self.assertEqual(ids0, ids1)
        data.update()
        ids2 = [id(f) for f in data._frames.values()]
        self.assertEqual(ids0, ids2)


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
