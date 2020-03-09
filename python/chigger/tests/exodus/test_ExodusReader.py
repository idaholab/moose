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
import shutil
import time
import mooseutils
import chigger

class TestExodusReader(unittest.TestCase):
    """
    Test use of MooseDataFrame for loading/reloading csv files.
    """
    @classmethod
    def setUpClass(cls):
        """
        Copy test files.
        """
        cls.single = "{}_single.e".format(cls.__name__)
        shutil.copyfile(os.path.abspath('../input/mug_blocks_out.e'), cls.single)

        cls.vector = "{}_vector.e".format(cls.__name__)
        shutil.copyfile(os.path.abspath('../input/vector_out.e'), cls.vector)

        cls.multiple = "{}_multiple".format(cls.__name__)
        cls.testfiles = chigger.utils.copy_adaptive_exodus_test_files(cls.multiple)
        cls.multiple += '.e'

    @classmethod
    def tearDownClass(cls):
        """
        Remove test files.
        """
        for fname in cls.testfiles:
            if os.path.exists(fname):
                os.remove(fname)
        if os.path.exists(cls.single):
            os.remove(cls.single)
        if os.path.exists(cls.vector):
            os.remove(cls.vector)

    def testSingle(self):
        """
        Test reading of a single Exodus file.
        """
        reader = chigger.exodus.ExodusReader(self.single)
        reader.update()

        # Times
        times = reader.getTimes()
        self.assertEqual(len(times), 21)
        self.assertEqual(times[0], 0)
        self.assertAlmostEqual(times[-1], 2)

        # Current Time
        reader.setOptions(timestep=None, time=1.01)
        if reader.needsUpdate():
            reader.update()
        tdata = reader.getTimeData()
        self.assertAlmostEqual(tdata.time, 1)
        self.assertEqual(tdata.timestep, 10)
        self.assertEqual(tdata.index, 10)
        self.assertEqual(tdata.filename, self.single)

        # Blocks
        blockinfo = reader.getBlockInformation()
        self.assertEqual(list(blockinfo[reader.BLOCK].keys()), ['1', '76'])
        self.assertEqual(list(blockinfo[reader.NODESET].keys()), ['1', '2'])
        self.assertEqual(list(blockinfo[reader.SIDESET].keys()), ['1', '2'])
        self.assertEqual(blockinfo[reader.SIDESET]['2'].name, 'top')
        self.assertEqual(blockinfo[reader.SIDESET]['2'].object_type, 3)
        self.assertEqual(blockinfo[reader.SIDESET]['2'].object_index, 1)
        self.assertEqual(blockinfo[reader.SIDESET]['2'].multiblock_index, 9)

        # Variable Info
        varinfo = reader.getVariableInformation()
        self.assertEqual(list(varinfo.keys()), ['aux_elem', 'convected', 'diffused', 'func_pp'])

        # Elemental Variables
        elemental = reader.getVariableInformation(var_types=[reader.ELEMENTAL])
        self.assertEqual(list(elemental.keys()), ['aux_elem'])
        self.assertEqual(elemental['aux_elem'].num_components, 1)

        # Nodal Variables
        elemental = reader.getVariableInformation(var_types=[reader.NODAL])
        self.assertEqual(list(elemental.keys()), ['convected', 'diffused'])
        self.assertEqual(elemental['diffused'].num_components, 1)

        # Global Variables
        gvars = reader.getVariableInformation(var_types=[reader.GLOBAL])
        self.assertEqual(list(gvars.keys()), ['func_pp'])
        self.assertEqual(gvars['func_pp'].num_components, 1)

    def testSingleFieldData(self):
        """
        Test that field data can be accessed.
        """
        reader = chigger.exodus.ExodusReader(self.single, variables=['func_pp'])
        for i, r in enumerate(range(0,21,2)):
            reader.update(timestep=i)
            self.assertAlmostEqual(reader.getGlobalData('func_pp'), r/10.)

    def testVector(self):
        """
        Test that vector data can be read.
        """
        reader = chigger.exodus.ExodusReader(self.vector)
        reader.update()

        variables = reader.getVariableInformation()
        self.assertEqual(list(variables.keys()), ['u', 'vel_'])
        self.assertEqual(variables['vel_'].num_components, 2)

    def testAdaptivity(self):
        """
        Test that adaptive timestep files load correctly.
        """
        reader = chigger.exodus.ExodusReader(self.multiple)
        reader.update()

        # Times
        self.assertEqual(reader.getTimes(), [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5])

        # Time
        reader.setOptions(timestep=None, time=1.01)
        if reader.needsUpdate():
            reader.update()
        tdata = reader.getTimeData()
        self.assertAlmostEqual(tdata.time, 1)
        self.assertEqual(tdata.timestep, 2)
        self.assertEqual(tdata.index, 0)
        self.assertEqual(tdata.filename, self.multiple + '-s002')

        # Wait and the "update" the first few files
        time.sleep(1.5)
        for i in range(6):
            mooseutils.touch(self.testfiles[i])

        reader.setOptions(time=None, timestep=-1)
        if reader.needsUpdate():
            reader.update()
        tdata = reader.getTimeData()
        self.assertEqual(reader.getTimes(), [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0])
        self.assertAlmostEqual(tdata.time, 3.0)
        self.assertEqual(tdata.timestep, 6)
        self.assertEqual(tdata.index, 0)
        self.assertEqual(tdata.filename, self.multiple + '-s006')

    def testExceptions(self):
        """
        Test for error messages.
        """

        # Invalid filename
        with self.assertRaisesRegexp(IOError, 'The file foo.e is not a valid filename.'):
            chigger.exodus.ExodusReader('foo.e')

        reader = chigger.exodus.ExodusReader(self.single, variables=['convected', 'func_pp'])
        with self.assertRaisesRegexp(mooseutils.MooseException, 'The variable "convected" must be a global variable.'):
            reader.getGlobalData('convected')

    def testReload(self):
        """
        Test the file reloading is working.
        """
        filenames = ['../input/diffusion_1.e', '../input/diffusion_2.e']
        common = 'common.e'
        shutil.copy(filenames[0], common)
        reader = chigger.exodus.ExodusReader(common)
        reader.update()
        self.assertEqual(reader.getVTKReader().GetNumberOfTimeSteps(), 2)

        shutil.copy(filenames[1], common)
        reader.update()
        self.assertEqual(reader.getVTKReader().GetNumberOfTimeSteps(), 3)

        shutil.copy(filenames[0], common)
        reader.update()
        self.assertEqual(reader.getVTKReader().GetNumberOfTimeSteps(), 2)



if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
