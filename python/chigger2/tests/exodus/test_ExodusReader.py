#!/usr/bin/env python
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
import logging
from moosetools import mooseutils
import chigger
import vtk

class TestExodusReader(unittest.TestCase):
    """
    Test use of ExodusReader.
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

        cls.interpolate = "{}_interpolate.e".format(cls.__name__)
        shutil.copyfile(os.path.abspath('../input/input_no_adapt_out.e'), cls.interpolate)

        cls.duplicate = "{}_duplicate.e".format(cls.__name__)
        shutil.copyfile(os.path.abspath('../input/duplicate_name_out.e'), cls.duplicate)

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
        if os.path.exists(cls.interpolate):
            os.remove(cls.interpolate)
        if os.path.exists(cls.duplicate):
            os.remove(cls.duplicate)

    def testCreateDelete(self):
        """Test that creating and deleting an object calls correct methods."""
        # Create ExodusReader
        with self.assertLogs(level=logging.DEBUG) as l:
            reader = chigger.exodus.ExodusReader(self.single)

        self.assertEqual(len(l.output), 2)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: setParams')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: setParams::Modified')

        # Delete the object
        with self.assertLogs(level=logging.DEBUG) as l:
            del reader
        self.assertEqual(len(l.output), 1)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: __del__()')

    def testParams(self):
        """Test the setting options calls the correct methods."""

        # Create ExodusReader
        reader = chigger.exodus.ExodusReader(self.single)

        # Test setParam
        with self.assertLogs(level=logging.DEBUG) as l:
            reader.setParam('timestep', 2)
            reader.updateInformation()

        self.assertEqual(len(l.output), 11)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: setParam')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: setParam::Modified')
        self.assertEqual(l.output[2], 'DEBUG:ExodusReader: updateInformation')
        self.assertEqual(l.output[3], 'DEBUG:ExodusReader: RequestInformation')
        self.assertEqual(l.output[4], 'DEBUG:ExodusReader: _onRequestInformation')
        self.assertEqual(l.output[5], 'DEBUG:ExodusReader: __updateFileInformation')
        self.assertEqual(l.output[6], 'DEBUG:ExodusReader: __updateTimeInformation')
        self.assertEqual(l.output[7], 'DEBUG:ExodusReader: __updateBlockInformation')
        self.assertEqual(l.output[8], 'DEBUG:ExodusReader: __updateVariableInformation')
        self.assertEqual(l.output[9], 'DEBUG:ExodusReader: __updateActiveBlocks')
        self.assertEqual(l.output[10], 'DEBUG:ExodusReader: __updateActiveVariables')

        # Test setParam, that doesn't change options
        with self.assertLogs(level=logging.DEBUG) as l:
            reader.setParam('timestep', 2)
            reader.updateInformation()

        self.assertEqual(len(l.output), 2)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: setParam')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: updateInformation')

        # Test setParams
        with self.assertLogs(level=logging.DEBUG) as l:
            reader.setParams(timestep=3)
            reader.updateInformation()

        self.assertEqual(len(l.output), 7)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: setParams')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: setParams::Modified')
        self.assertEqual(l.output[2], 'DEBUG:ExodusReader: updateInformation')
        self.assertEqual(l.output[3], 'DEBUG:ExodusReader: RequestInformation')
        self.assertEqual(l.output[4], 'DEBUG:ExodusReader: _onRequestInformation')
        self.assertEqual(l.output[5], 'DEBUG:ExodusReader: __updateActiveBlocks')
        self.assertEqual(l.output[6], 'DEBUG:ExodusReader: __updateActiveVariables')

        # Test setParams, that doesn't change options
        with self.assertLogs(level=logging.DEBUG) as l:
            reader.setParams(timestep=3)
            reader.updateInformation()

        self.assertEqual(len(l.output), 2)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: setParams')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: updateInformation')


    def testFileInformation(self):
        """Test that the file information is updated correctly."""

        # Create ExodusReader
        reader = chigger.exodus.ExodusReader(self.single)

        # Get FileInfo dict, this should trigger a call via the VTK pipeline RequestInformation
        with self.assertLogs(level=logging.DEBUG) as l:
            finfo = reader.getFileInformation()

        self.assertEqual(len(l.output), 9)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: updateInformation')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: RequestInformation')
        self.assertEqual(l.output[2], 'DEBUG:ExodusReader: _onRequestInformation')
        self.assertEqual(l.output[3], 'DEBUG:ExodusReader: __updateFileInformation')
        self.assertEqual(l.output[4], 'DEBUG:ExodusReader: __updateTimeInformation')
        self.assertEqual(l.output[5], 'DEBUG:ExodusReader: __updateBlockInformation')
        self.assertEqual(l.output[6], 'DEBUG:ExodusReader: __updateVariableInformation')
        self.assertEqual(l.output[7], 'DEBUG:ExodusReader: __updateActiveBlocks')
        self.assertEqual(l.output[8], 'DEBUG:ExodusReader: __updateActiveVariables')

        # Get FileInfo dict, this should NOT trigger a call to the VTK pipeline RequestInformation
        with self.assertLogs(level=logging.DEBUG) as l:
            finfo = reader.getFileInformation()

        self.assertEqual(len(l.output), 1)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: updateInformation')

        # Delete the object
        with self.assertLogs(level=logging.DEBUG) as l:
            del reader
        self.assertEqual(len(l.output), 1)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: __del__()')

        # Check that the file information is valid
        self.assertEqual(len(finfo), 1)
        self.assertIn(self.single, finfo)

        f = finfo[self.single]
        self.assertEqual(f.filename, self.single)
        self.assertEqual(len(f.times), 21)

    def testTimeInformation(self):
        """Test the TimeInfo objects"""

        # Create ExodusReader
        reader = chigger.exodus.ExodusReader(self.single)

        # Get TimeInfo list, this should trigger a call via the VTK pipeline RequestInformation
        with self.assertLogs(level=logging.DEBUG) as l:
            tinfo = reader.getTimeInformation()

        self.assertEqual(len(l.output), 9)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: updateInformation')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: RequestInformation')
        self.assertEqual(l.output[2], 'DEBUG:ExodusReader: _onRequestInformation')
        self.assertEqual(l.output[3], 'DEBUG:ExodusReader: __updateFileInformation')
        self.assertEqual(l.output[4], 'DEBUG:ExodusReader: __updateTimeInformation')
        self.assertEqual(l.output[5], 'DEBUG:ExodusReader: __updateBlockInformation')
        self.assertEqual(l.output[6], 'DEBUG:ExodusReader: __updateVariableInformation')
        self.assertEqual(l.output[7], 'DEBUG:ExodusReader: __updateActiveBlocks')
        self.assertEqual(l.output[8], 'DEBUG:ExodusReader: __updateActiveVariables')

        # Get TimeInfo list, this should NOT trigger a call to the VTK pipeline RequestInformation
        with self.assertLogs(level=logging.DEBUG) as l:
            tinfo = reader.getTimeInformation()

        self.assertEqual(len(l.output), 1)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: updateInformation')

        self.assertEqual(len(tinfo), 21)
        self.assertEqual(tinfo[0].timestep, 0)
        self.assertEqual(tinfo[0].time, 0.0)
        self.assertEqual(tinfo[0].filename, self.single)
        self.assertEqual(tinfo[0].index, 0)

        self.assertEqual(tinfo[9].timestep, 9)
        self.assertAlmostEqual(tinfo[9].time, 0.9)
        self.assertEqual(tinfo[9].filename, self.single)
        self.assertEqual(tinfo[9].index, 9)

    def testCurrentTimeInformation(self):
        """Test the returning of the current TimeInfo objects"""

        # Create ExodusReader
        reader = chigger.exodus.ExodusReader(self.single)

        # Get TimeInfo tuple, this should trigger a call via the VTK pipeline RequestInformation
        with self.assertLogs(level=logging.DEBUG) as l:
            tinfo = reader.getCurrentTimeInformation()

        self.assertEqual(len(l.output), 9)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: updateInformation')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: RequestInformation')
        self.assertEqual(l.output[2], 'DEBUG:ExodusReader: _onRequestInformation')
        self.assertEqual(l.output[3], 'DEBUG:ExodusReader: __updateFileInformation')
        self.assertEqual(l.output[4], 'DEBUG:ExodusReader: __updateTimeInformation')
        self.assertEqual(l.output[5], 'DEBUG:ExodusReader: __updateBlockInformation')
        self.assertEqual(l.output[6], 'DEBUG:ExodusReader: __updateVariableInformation')
        self.assertEqual(l.output[7], 'DEBUG:ExodusReader: __updateActiveBlocks')
        self.assertEqual(l.output[8], 'DEBUG:ExodusReader: __updateActiveVariables')

        # Get TimeInfo tuple, this should NOT trigger a call to the VTK pipeline RequestInformation
        with self.assertLogs(level=logging.DEBUG) as l:
            tinfo = reader.getCurrentTimeInformation()

        self.assertEqual(len(l.output), 1)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: updateInformation')

        self.assertEqual(len(tinfo), 2)
        self.assertIsNone(tinfo[1])
        self.assertEqual(tinfo[0].timestep, 20)
        self.assertAlmostEqual(tinfo[0].time, 2)
        self.assertEqual(tinfo[0].filename, self.single)
        self.assertEqual(tinfo[0].index, 20)

        # Change the time, the __update* methods should NOT be called because the files didn't change
        reader.setParam('time', 1)
        with self.assertLogs(level=logging.DEBUG) as l:
            tinfo = reader.getCurrentTimeInformation()

        self.assertEqual(len(l.output), 5)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: updateInformation')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: RequestInformation')
        self.assertEqual(l.output[2], 'DEBUG:ExodusReader: _onRequestInformation')
        self.assertEqual(l.output[3], 'DEBUG:ExodusReader: __updateActiveBlocks')
        self.assertEqual(l.output[4], 'DEBUG:ExodusReader: __updateActiveVariables')

        self.assertEqual(len(tinfo), 2)
        self.assertEqual(tinfo[0].timestep, 10)
        self.assertAlmostEqual(tinfo[0].time, 1)
        self.assertEqual(tinfo[0].filename, self.single)
        self.assertEqual(tinfo[0].index, 10)

        self.assertEqual(tinfo[1].timestep, 11)
        self.assertAlmostEqual(tinfo[1].time, 1.1)
        self.assertEqual(tinfo[1].filename, self.single)
        self.assertEqual(tinfo[1].index, 11)

    def testBlockInformation(self):
        """Test the BlockInfo objects"""

        # Create ExodusReader
        with self.assertLogs(level=logging.DEBUG) as l:
            reader = chigger.exodus.ExodusReader(self.single)

        # Get BlockInfo dict, this should trigger a call via the VTK pipeline RequestInformation
        with self.assertLogs(level=logging.DEBUG) as l:
            binfo = reader.getBlockInformation()

        self.assertEqual(len(l.output), 9)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: updateInformation')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: RequestInformation')
        self.assertEqual(l.output[2], 'DEBUG:ExodusReader: _onRequestInformation')
        self.assertEqual(l.output[3], 'DEBUG:ExodusReader: __updateFileInformation')
        self.assertEqual(l.output[4], 'DEBUG:ExodusReader: __updateTimeInformation')
        self.assertEqual(l.output[5], 'DEBUG:ExodusReader: __updateBlockInformation')
        self.assertEqual(l.output[6], 'DEBUG:ExodusReader: __updateVariableInformation')
        self.assertEqual(l.output[7], 'DEBUG:ExodusReader: __updateActiveBlocks')
        self.assertEqual(l.output[8], 'DEBUG:ExodusReader: __updateActiveVariables')

        # Get BlockInfo dict, this should trigger a call via the VTK pipeline RequestInformation
        with self.assertLogs(level=logging.DEBUG) as l:
            binfo = reader.getBlockInformation()

        self.assertEqual(len(l.output), 1)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: updateInformation')

        self.assertEqual(len(binfo), 3)

        blk_binfo = binfo[chigger.exodus.ExodusReader.BLOCK]
        self.assertEqual(len(blk_binfo), 2)
        self.assertEqual(blk_binfo[0].name, '1')
        self.assertEqual(blk_binfo[0].number, 1)
        self.assertEqual(blk_binfo[0].object_type, chigger.exodus.ExodusReader.BLOCK)
        self.assertEqual(blk_binfo[0].object_index, 0)
        self.assertEqual(blk_binfo[0].multiblock_index, 2)
        self.assertTrue(blk_binfo[0].active)

        self.assertEqual(blk_binfo[1].name, '76')
        self.assertEqual(blk_binfo[1].number, 76)
        self.assertEqual(blk_binfo[1].object_type, chigger.exodus.ExodusReader.BLOCK)
        self.assertEqual(blk_binfo[1].object_index, 1)
        self.assertEqual(blk_binfo[1].multiblock_index, 3)
        self.assertTrue(blk_binfo[1].active)

        blk_binfo = binfo[chigger.exodus.ExodusReader.SIDESET]
        self.assertEqual(len(blk_binfo), 2)
        self.assertEqual(blk_binfo[0].name, 'bottom')
        self.assertEqual(blk_binfo[0].number, 1)
        self.assertEqual(blk_binfo[0].object_type, chigger.exodus.ExodusReader.SIDESET)
        self.assertEqual(blk_binfo[0].object_index, 0)
        self.assertEqual(blk_binfo[0].multiblock_index, 8)
        self.assertTrue(blk_binfo[0].active)

        self.assertEqual(blk_binfo[1].name, 'top')
        self.assertEqual(blk_binfo[1].number, 2)
        self.assertEqual(blk_binfo[1].object_type, chigger.exodus.ExodusReader.SIDESET)
        self.assertEqual(blk_binfo[1].object_index, 1)
        self.assertEqual(blk_binfo[1].multiblock_index, 9)
        self.assertTrue(blk_binfo[1].active)

        blk_binfo = binfo[chigger.exodus.ExodusReader.NODESET]
        self.assertEqual(len(blk_binfo), 2)
        self.assertEqual(blk_binfo[0].name, '1')
        self.assertEqual(blk_binfo[0].number, 1)
        self.assertEqual(blk_binfo[0].object_type, chigger.exodus.ExodusReader.NODESET)
        self.assertEqual(blk_binfo[0].object_index, 0)
        self.assertEqual(blk_binfo[0].multiblock_index, 13)
        self.assertTrue(blk_binfo[0].active)

        self.assertEqual(blk_binfo[1].name, '2')
        self.assertEqual(blk_binfo[1].number, 2)
        self.assertEqual(blk_binfo[1].object_type, chigger.exodus.ExodusReader.NODESET)
        self.assertEqual(blk_binfo[1].object_index, 1)
        self.assertEqual(blk_binfo[1].multiblock_index, 14)
        self.assertTrue(blk_binfo[1].active)

        # Test setting 'blocks'
        with self.assertLogs(level=logging.DEBUG) as l:
            reader.setParams(blocks=(1,))
            binfo = reader.getBlockInformation()

        self.assertEqual(len(l.output), 7)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: setParams')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: setParams::Modified')
        self.assertEqual(l.output[2], 'DEBUG:ExodusReader: updateInformation')
        self.assertEqual(l.output[3], 'DEBUG:ExodusReader: RequestInformation')
        self.assertEqual(l.output[4], 'DEBUG:ExodusReader: _onRequestInformation')
        self.assertEqual(l.output[5], 'DEBUG:ExodusReader: __updateActiveBlocks')
        self.assertEqual(l.output[6], 'DEBUG:ExodusReader: __updateActiveVariables')

        blk_binfo = binfo[chigger.exodus.ExodusReader.BLOCK]
        self.assertTrue(blk_binfo[0].active)
        self.assertFalse(blk_binfo[1].active)

        blk_binfo = binfo[chigger.exodus.ExodusReader.SIDESET]
        self.assertFalse(blk_binfo[0].active)
        self.assertFalse(blk_binfo[1].active)

        blk_binfo = binfo[chigger.exodus.ExodusReader.NODESET]
        self.assertFalse(blk_binfo[0].active)
        self.assertFalse(blk_binfo[1].active)

        with self.assertLogs(level=logging.DEBUG) as l:
            reader.updateData()

        self.assertEqual(len(l.output), 3)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: updateData')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: RequestData')
        self.assertEqual(l.output[2], 'DEBUG:ExodusReader: _onRequestData')

        tinfo, _ = reader.getCurrentTimeInformation()
        vtkreader = reader.getFileInformation()[tinfo.filename].vtkreader
        self.assertTrue(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.ELEM_BLOCK, 0))
        self.assertFalse(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.ELEM_BLOCK, 1))
        self.assertFalse(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.SIDE_SET, 0))
        self.assertFalse(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.SIDE_SET, 1))
        self.assertFalse(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.NODE_SET, 0))
        self.assertFalse(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.NODE_SET, 1))

        # Test setting 'nodesets'
        with self.assertLogs(level=logging.DEBUG) as l:
            reader.setParams(nodesets=(2,))
            binfo = reader.getBlockInformation()

        self.assertEqual(len(l.output), 7)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: setParams')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: setParams::Modified')
        self.assertEqual(l.output[2], 'DEBUG:ExodusReader: updateInformation')
        self.assertEqual(l.output[3], 'DEBUG:ExodusReader: RequestInformation')
        self.assertEqual(l.output[4], 'DEBUG:ExodusReader: _onRequestInformation')
        self.assertEqual(l.output[5], 'DEBUG:ExodusReader: __updateActiveBlocks')
        self.assertEqual(l.output[6], 'DEBUG:ExodusReader: __updateActiveVariables')

        blk_binfo = binfo[chigger.exodus.ExodusReader.BLOCK]
        self.assertTrue(blk_binfo[0].active)
        self.assertFalse(blk_binfo[1].active)

        blk_binfo = binfo[chigger.exodus.ExodusReader.SIDESET]
        self.assertFalse(blk_binfo[0].active)
        self.assertFalse(blk_binfo[1].active)

        blk_binfo = binfo[chigger.exodus.ExodusReader.NODESET]
        self.assertFalse(blk_binfo[0].active)
        self.assertTrue(blk_binfo[1].active)

        with self.assertLogs(level=logging.DEBUG) as l:
            reader.updateData()

        self.assertTrue(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.ELEM_BLOCK, 0))
        self.assertFalse(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.ELEM_BLOCK, 1))
        self.assertFalse(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.SIDE_SET, 0))
        self.assertFalse(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.SIDE_SET, 1))
        self.assertFalse(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.NODE_SET, 0))
        self.assertTrue(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.NODE_SET, 1))

        # Test setting 'sidesets'
        with self.assertLogs(level=logging.DEBUG) as l:
            reader.setParams(sidesets=('bottom',))
            binfo = reader.getBlockInformation()

        self.assertEqual(len(l.output), 7)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: setParams')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: setParams::Modified')
        self.assertEqual(l.output[2], 'DEBUG:ExodusReader: updateInformation')
        self.assertEqual(l.output[3], 'DEBUG:ExodusReader: RequestInformation')
        self.assertEqual(l.output[4], 'DEBUG:ExodusReader: _onRequestInformation')
        self.assertEqual(l.output[5], 'DEBUG:ExodusReader: __updateActiveBlocks')
        self.assertEqual(l.output[6], 'DEBUG:ExodusReader: __updateActiveVariables')

        blk_binfo = binfo[chigger.exodus.ExodusReader.BLOCK]
        self.assertTrue(blk_binfo[0].active)
        self.assertFalse(blk_binfo[1].active)

        blk_binfo = binfo[chigger.exodus.ExodusReader.SIDESET]
        self.assertTrue(blk_binfo[0].active)
        self.assertFalse(blk_binfo[1].active)

        blk_binfo = binfo[chigger.exodus.ExodusReader.NODESET]
        self.assertFalse(blk_binfo[0].active)
        self.assertTrue(blk_binfo[1].active)

        with self.assertLogs(level=logging.DEBUG) as l:
            reader.updateData()

        self.assertTrue(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.ELEM_BLOCK, 0))
        self.assertFalse(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.ELEM_BLOCK, 1))
        self.assertTrue(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.SIDE_SET, 0))
        self.assertFalse(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.SIDE_SET, 1))
        self.assertFalse(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.NODE_SET, 0))
        self.assertTrue(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.NODE_SET, 1))

        # Test un-setting
        with self.assertLogs(level=logging.DEBUG) as l:
            reader.setParams(blocks=tuple(), sidesets=tuple(), nodesets=tuple())
            binfo = reader.getBlockInformation()
            reader.updateData()

        self.assertEqual(len(l.output), 10)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: setParams')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: setParams::Modified')
        self.assertEqual(l.output[2], 'DEBUG:ExodusReader: updateInformation')
        self.assertEqual(l.output[3], 'DEBUG:ExodusReader: RequestInformation')
        self.assertEqual(l.output[4], 'DEBUG:ExodusReader: _onRequestInformation')
        self.assertEqual(l.output[5], 'DEBUG:ExodusReader: __updateActiveBlocks')
        self.assertEqual(l.output[6], 'DEBUG:ExodusReader: __updateActiveVariables')
        self.assertEqual(l.output[7], 'DEBUG:ExodusReader: updateData')
        self.assertEqual(l.output[8], 'DEBUG:ExodusReader: RequestData')
        self.assertEqual(l.output[9], 'DEBUG:ExodusReader: _onRequestData')

        blk_binfo = binfo[chigger.exodus.ExodusReader.BLOCK]
        self.assertTrue(blk_binfo[0].active)
        self.assertTrue(blk_binfo[1].active)

        blk_binfo = binfo[chigger.exodus.ExodusReader.SIDESET]
        self.assertTrue(blk_binfo[0].active)
        self.assertTrue(blk_binfo[1].active)

        blk_binfo = binfo[chigger.exodus.ExodusReader.NODESET]
        self.assertTrue(blk_binfo[0].active)
        self.assertTrue(blk_binfo[1].active)

        self.assertTrue(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.ELEM_BLOCK, 0))
        self.assertTrue(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.ELEM_BLOCK, 1))
        self.assertTrue(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.SIDE_SET, 0))
        self.assertTrue(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.SIDE_SET, 1))
        self.assertTrue(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.NODE_SET, 0))
        self.assertTrue(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.NODE_SET, 1))

        # Test disable
        with self.assertLogs(level=logging.DEBUG) as l:
            reader.setParams(blocks=None, sidesets=tuple(), nodesets=None)
            binfo = reader.getBlockInformation()
            reader.updateData()

        self.assertEqual(len(l.output), 10)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: setParams')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: setParams::Modified')
        self.assertEqual(l.output[2], 'DEBUG:ExodusReader: updateInformation')
        self.assertEqual(l.output[3], 'DEBUG:ExodusReader: RequestInformation')
        self.assertEqual(l.output[4], 'DEBUG:ExodusReader: _onRequestInformation')
        self.assertEqual(l.output[5], 'DEBUG:ExodusReader: __updateActiveBlocks')
        self.assertEqual(l.output[6], 'DEBUG:ExodusReader: __updateActiveVariables')
        self.assertEqual(l.output[7], 'DEBUG:ExodusReader: updateData')
        self.assertEqual(l.output[8], 'DEBUG:ExodusReader: RequestData')
        self.assertEqual(l.output[9], 'DEBUG:ExodusReader: _onRequestData')

        blk_binfo = binfo[chigger.exodus.ExodusReader.BLOCK]
        self.assertFalse(blk_binfo[0].active)
        self.assertFalse(blk_binfo[1].active)

        blk_binfo = binfo[chigger.exodus.ExodusReader.SIDESET]
        self.assertTrue(blk_binfo[0].active)
        self.assertTrue(blk_binfo[1].active)

        blk_binfo = binfo[chigger.exodus.ExodusReader.NODESET]
        self.assertFalse(blk_binfo[0].active)
        self.assertFalse(blk_binfo[1].active)

        self.assertFalse(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.ELEM_BLOCK, 0))
        self.assertFalse(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.ELEM_BLOCK, 1))
        self.assertTrue(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.SIDE_SET, 0))
        self.assertTrue(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.SIDE_SET, 1))
        self.assertFalse(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.NODE_SET, 0))
        self.assertFalse(vtkreader.GetObjectStatus(vtk.vtkExodusIIReader.NODE_SET, 1))

    def testWarnings(self):
        with self.assertLogs() as l:
            reader = chigger.exodus.ExodusReader(self.single, time=1000)
            reader.updateInformation()
            reader.updateData()
        self.assertIn("Time out of range, 1000.0 not in", l.output[0])

        with self.assertLogs() as l:
            reader = chigger.exodus.ExodusReader(self.single, timestep=-2)
            reader.updateInformation()
            reader.updateData()
        self.assertIn("Timestep out of range: -2 not in", l.output[0])

        with self.assertLogs() as l:
            reader = chigger.exodus.ExodusReader(self.single, timestep=42)
            reader.updateInformation()
            reader.updateData()
        self.assertIn("Timestep out of range: 42 not in", l.output[0])

        with self.assertLogs() as l:
            reader = chigger.exodus.ExodusReader(self.single, blocks=['nope'])
            reader.updateInformation()
        self.assertIn("The following items in 'blocks' do not exist: nope", l.output[0])

        with self.assertLogs() as l:
            reader = chigger.exodus.ExodusReader(self.single, nodesets=['nope'])
            reader.updateInformation()
        self.assertIn("The following items in 'nodesets' do not exist: nope", l.output[0])

        with self.assertLogs() as l:
            reader = chigger.exodus.ExodusReader(self.single, sidesets=['nope'])
            reader.updateInformation()
        self.assertIn("The following items in 'sidesets' do not exist: nope", l.output[0])

        with self.assertLogs() as l:
            reader = chigger.exodus.ExodusReader(self.duplicate, variables=('variable',))
            reader.updateInformation()
        self.assertIn("The variable name 'variable' exists with multiple", l.output[0])

    def testErrors(self):
        """
        Test for error messages.
        """
        # Invalid filename
        with self.assertLogs(level=logging.ERROR) as l:
            reader = chigger.exodus.ExodusReader('foo.e')
            reader.updateInformation()
        self.assertIn("The file foo.e is not a valid filename.", l.output[0])

        with self.assertLogs() as l:
            reader = chigger.exodus.ExodusReader(self.single, variables=('convected', 'func_pp'))
            reader.getGlobalData('convected')
        self.assertIn("The supplied global variable, 'convected', does not ", l.output[0])

        with self.assertLogs() as l:
            reader.setParams(variables=('convected::WRONG',))
            reader.updateInformation()
        self.assertIn("Unknown variable prefix '::WRONG'", l.output[0])

        with self.assertLogs() as l:
            reader = chigger.exodus.ExodusReader(self.multiple, time=1.12345)
            reader.updateInformation()
            reader.updateData()
        self.assertIn("Support for time interpolation across adaptive time steps is not supported.", l.output[0])

    def testVariableInformation(self):
        """Test the VarInfo objects"""

        # Create ExodusReader
        reader = chigger.exodus.ExodusReader(self.single)

        # Get VarInfo dict, this should trigger a call via the VTK pipeline RequestInformation
        with self.assertLogs(level=logging.DEBUG) as l:
            vinfo = reader.getVariableInformation()

        self.assertEqual(len(l.output), 9)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: updateInformation')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: RequestInformation')
        self.assertEqual(l.output[2], 'DEBUG:ExodusReader: _onRequestInformation')
        self.assertEqual(l.output[3], 'DEBUG:ExodusReader: __updateFileInformation')
        self.assertEqual(l.output[4], 'DEBUG:ExodusReader: __updateTimeInformation')
        self.assertEqual(l.output[5], 'DEBUG:ExodusReader: __updateBlockInformation')
        self.assertEqual(l.output[6], 'DEBUG:ExodusReader: __updateVariableInformation')
        self.assertEqual(l.output[7], 'DEBUG:ExodusReader: __updateActiveBlocks')
        self.assertEqual(l.output[8], 'DEBUG:ExodusReader: __updateActiveVariables')

        # Get VariableInfo list, this should trigger a call via the VTK pipeline RequestInformation
        with self.assertLogs(level=logging.DEBUG) as l:
            vinfo = reader.getVariableInformation()

        self.assertEqual(len(l.output), 1)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: updateInformation')

        self.assertEqual(len(vinfo), 3)

        e_vinfo = vinfo[chigger.exodus.ExodusReader.ELEMENTAL]
        self.assertEqual(len(e_vinfo), 1)
        self.assertEqual(e_vinfo[0].name, 'aux_elem')
        self.assertEqual(e_vinfo[0].object_type, chigger.exodus.ExodusReader.ELEMENTAL)
        self.assertEqual(e_vinfo[0].num_components, 1)
        self.assertTrue(e_vinfo[0].active)

        n_vinfo = vinfo[chigger.exodus.ExodusReader.NODAL]
        self.assertEqual(len(n_vinfo), 2)
        self.assertEqual(n_vinfo[0].name, 'convected')
        self.assertEqual(n_vinfo[0].object_type, chigger.exodus.ExodusReader.NODAL)
        self.assertEqual(n_vinfo[0].num_components, 1)
        self.assertTrue(n_vinfo[0].active)

        self.assertEqual(n_vinfo[1].name, 'diffused')
        self.assertEqual(n_vinfo[1].object_type, chigger.exodus.ExodusReader.NODAL)
        self.assertEqual(n_vinfo[1].num_components, 1)
        self.assertTrue(n_vinfo[1].active)

        g_vinfo = vinfo[chigger.exodus.ExodusReader.GLOBAL]
        self.assertEqual(len(g_vinfo), 1)
        self.assertEqual(g_vinfo[0].name, 'func_pp')
        self.assertEqual(g_vinfo[0].object_type, chigger.exodus.ExodusReader.GLOBAL)
        self.assertEqual(g_vinfo[0].num_components, 1)
        self.assertTrue(g_vinfo[0].active)

        # Test 'variables=...'
        with self.assertLogs(level=logging.DEBUG) as l:
            reader.setParams(variables=('convected',))
            vinfo = reader.getVariableInformation()

        self.assertEqual(len(l.output), 7)
        self.assertEqual(l.output[0], 'DEBUG:ExodusReader: setParams')
        self.assertEqual(l.output[1], 'DEBUG:ExodusReader: setParams::Modified')
        self.assertEqual(l.output[2], 'DEBUG:ExodusReader: updateInformation')
        self.assertEqual(l.output[3], 'DEBUG:ExodusReader: RequestInformation')
        self.assertEqual(l.output[4], 'DEBUG:ExodusReader: _onRequestInformation')
        self.assertEqual(l.output[5], 'DEBUG:ExodusReader: __updateActiveBlocks')
        self.assertEqual(l.output[6], 'DEBUG:ExodusReader: __updateActiveVariables')

        e_vinfo = vinfo[chigger.exodus.ExodusReader.ELEMENTAL]
        self.assertFalse(e_vinfo[0].active)

        n_vinfo = vinfo[chigger.exodus.ExodusReader.NODAL]
        self.assertTrue(n_vinfo[0].active)
        self.assertFalse(n_vinfo[1].active)

        g_vinfo = vinfo[chigger.exodus.ExodusReader.GLOBAL]
        self.assertFalse(g_vinfo[0].active)

    def testSingle(self):
        """
        Test reading of a single Exodus file, with default options
        """
        reader = chigger.exodus.ExodusReader(self.single)

        # Times
        times = reader.getTimes()
        self.assertEqual(len(times), 21)
        self.assertEqual(times[0], 0)
        self.assertAlmostEqual(times[-1], 2)

        # Current Time
        reader.setParams(timestep=None, time=1.01)
        tdata0, tdata1 = reader.getCurrentTimeInformation()
        self.assertAlmostEqual(tdata0.time, 1)
        self.assertEqual(tdata0.timestep, 10)
        self.assertEqual(tdata0.index, 10)
        self.assertEqual(tdata0.filename, self.single)

        self.assertAlmostEqual(tdata1.time, 1.1)
        self.assertEqual(tdata1.timestep, 11)
        self.assertEqual(tdata1.index, 11)
        self.assertEqual(tdata1.filename, self.single)

        # Blocks
        blockinfo = reader.getBlockInformation()
        self.assertEqual([b.name for b in blockinfo[reader.BLOCK]], ['1', '76',])
        self.assertEqual([b.name for b in blockinfo[reader.NODESET]], ['1', '2'])
        self.assertEqual([b.name for b in blockinfo[reader.SIDESET]], ['bottom', 'top'])
        self.assertEqual(blockinfo[reader.SIDESET][1].name, 'top')
        self.assertEqual(blockinfo[reader.SIDESET][1].object_type, 3)
        self.assertEqual(blockinfo[reader.SIDESET][1].object_index, 1)
        self.assertEqual(blockinfo[reader.SIDESET][1].multiblock_index, 9)

        # Variable Info
        varinfo = reader.getVariableInformation()
        self.assertEqual([v.name for v in varinfo[reader.ELEMENTAL]], ['aux_elem'])
        self.assertEqual([v.name for v in varinfo[reader.NODAL]], ['convected', 'diffused'])
        self.assertEqual([v.name for v in varinfo[reader.GLOBAL]], ['func_pp'])

    def testSingleNoInterpolation(self):
        """
        Test reading of a single Exodus file, without time interpolation
        """
        reader = chigger.exodus.ExodusReader(self.single)

        # Times
        times = reader.getTimes()
        self.assertEqual(len(times), 21)
        self.assertEqual(times[0], 0)
        self.assertAlmostEqual(times[-1], 2)

        # Current Time
        reader.setParams(timestep=None, time=1.01, time_interpolation=False)
        tdata0, tdata1 = reader.getCurrentTimeInformation()

        self.assertAlmostEqual(tdata0.time, 1)
        self.assertEqual(tdata0.timestep, 10)
        self.assertEqual(tdata0.index, 10)
        self.assertEqual(tdata0.filename, self.single)

        self.assertIsNone(tdata1)

    def testTimeInterpolation(self):
        reader = chigger.exodus.ExodusReader(self.interpolate)

        reader.updateInformation()
        reader.updateData()
        data = reader.GetOutputDataObject(0).GetBlock(0).GetBlock(0)

        # Time = 10
        cdata = data.GetCellData()
        self.assertEqual(cdata.GetNumberOfArrays(), 2)
        self.assertEqual(cdata.GetNumberOfComponents(), 2)
        self.assertEqual(cdata.GetNumberOfTuples(), 1600)
        self.assertEqual(cdata.GetArray(0).GetName(), 'elemental')
        self.assertEqual(cdata.GetArray(1).GetName(), 'ObjectId')
        rng = cdata.GetArray(0).GetRange()
        self.assertAlmostEqual(rng[0], -9.93844170297569)
        self.assertAlmostEqual(rng[1], 9.93844170297569)

        ndata = data.GetPointData()
        self.assertEqual(ndata.GetNumberOfArrays(), 1)
        self.assertEqual(ndata.GetNumberOfComponents(), 1)
        self.assertEqual(ndata.GetNumberOfTuples(), 1681)
        self.assertEqual(ndata.GetArray(0).GetName(), 'nodal')
        rng = ndata.GetArray(0).GetRange()
        self.assertAlmostEqual(rng[0], -10)
        self.assertAlmostEqual(rng[1], 10)

        gdata = data.GetFieldData()
        self.assertEqual(gdata.GetNumberOfArrays(), 4)
        self.assertEqual(gdata.GetArray(0).GetName(), 'global')
        self.assertEqual(gdata.GetArray(0).GetComponent(2, 0), 10)

        # Time = 5
        reader.setParams(time=5)
        reader.Update()
        data = reader.GetOutputDataObject(0).GetBlock(0).GetBlock(0)
        cdata = data.GetCellData()
        self.assertEqual(cdata.GetNumberOfArrays(), 2)
        self.assertEqual(cdata.GetNumberOfComponents(), 2)
        self.assertEqual(cdata.GetNumberOfTuples(), 1600)
        self.assertEqual(cdata.GetArray(0).GetName(), 'elemental')
        self.assertEqual(cdata.GetArray(1).GetName(), 'ObjectId')
        rng = cdata.GetArray(0).GetRange()
        self.assertAlmostEqual(rng[0], -4.969220851487845)
        self.assertAlmostEqual(rng[1], 4.969220851487845)

        ndata = data.GetPointData()
        self.assertEqual(ndata.GetNumberOfArrays(), 1)
        self.assertEqual(ndata.GetNumberOfComponents(), 1)
        self.assertEqual(ndata.GetNumberOfTuples(), 1681)
        self.assertEqual(ndata.GetArray(0).GetName(), 'nodal')
        rng = ndata.GetArray(0).GetRange()
        self.assertAlmostEqual(rng[0], -5)
        self.assertAlmostEqual(rng[1], 5)

        gdata = data.GetFieldData()
        self.assertEqual(gdata.GetNumberOfArrays(), 4)
        self.assertEqual(gdata.GetArray(0).GetName(), 'global')
        self.assertEqual(gdata.GetArray(0).GetComponent(1, 0), 5)

        # Time = 7.5
        reader.setParams(time=7.5)
        reader.Update()
        data = reader.GetOutputDataObject(0).GetBlock(0).GetBlock(0)
        cdata = data.GetCellData()
        self.assertEqual(cdata.GetNumberOfArrays(), 2)
        self.assertEqual(cdata.GetNumberOfComponents(), 2)
        self.assertEqual(cdata.GetNumberOfTuples(), 1600)
        self.assertEqual(cdata.GetArray(0).GetName(), 'elemental')
        self.assertEqual(cdata.GetArray(1).GetName(), 'ObjectId')
        rng = cdata.GetArray(0).GetRange()
        self.assertAlmostEqual(rng[0], -7.453831277231767)
        self.assertAlmostEqual(rng[1], 7.453831277231767)

        ndata = data.GetPointData()
        self.assertEqual(ndata.GetNumberOfArrays(), 1)
        self.assertEqual(ndata.GetNumberOfComponents(), 1)
        self.assertEqual(ndata.GetNumberOfTuples(), 1681)
        self.assertEqual(ndata.GetArray(0).GetName(), 'nodal')
        rng = ndata.GetArray(0).GetRange()
        self.assertAlmostEqual(rng[0], -7.5)
        self.assertAlmostEqual(rng[1], 7.5)

        self.assertEqual(reader.getGlobalData('global'), 7.5)

        # Time = 2.25
        reader.setParams(time=2.25)
        reader.Update()
        data = reader.GetOutputDataObject(0).GetBlock(0).GetBlock(0)
        cdata = data.GetCellData()
        self.assertEqual(cdata.GetNumberOfArrays(), 2)
        self.assertEqual(cdata.GetNumberOfComponents(), 2)
        self.assertEqual(cdata.GetNumberOfTuples(), 1600)
        self.assertEqual(cdata.GetArray(0).GetName(), 'elemental')
        self.assertEqual(cdata.GetArray(1).GetName(), 'ObjectId')
        rng = cdata.GetArray(0).GetRange()
        self.assertAlmostEqual(rng[0], -2.23614938316953)
        self.assertAlmostEqual(rng[1], 2.23614938316953)

        ndata = data.GetPointData()
        self.assertEqual(ndata.GetNumberOfArrays(), 1)
        self.assertEqual(ndata.GetNumberOfComponents(), 1)
        self.assertEqual(ndata.GetNumberOfTuples(), 1681)
        self.assertEqual(ndata.GetArray(0).GetName(), 'nodal')
        rng = ndata.GetArray(0).GetRange()
        self.assertAlmostEqual(rng[0], -2.25)
        self.assertAlmostEqual(rng[1], 2.25)

        self.assertEqual(reader.getGlobalData('global'), 2.25)

    def testSingleFieldData(self):
        """
        Test that field data can be accessed.
        """
        reader = chigger.exodus.ExodusReader(self.single, variables=('func_pp',))
        for i, r in enumerate(range(0,21,2)):
            reader.setParams(timestep=i)
            self.assertAlmostEqual(reader.getGlobalData('func_pp'), r/10.)

    def testVector(self):
        """
        Test that vector data can be read.
        """
        reader = chigger.exodus.ExodusReader(self.vector)
        variables = reader.getVariableInformation()
        self.assertEqual([v.name for v in variables[chigger.exodus.ExodusReader.NODAL]], ['u', 'vel_'])
        self.assertEqual(variables[chigger.exodus.ExodusReader.NODAL][1].num_components, 2)

    def testAdaptivity(self):
        """
        Test that adaptive timestep files load correctly.
        """
        reader = chigger.exodus.ExodusReader(self.multiple, time_interpolation=False)

        # Times
        self.assertEqual(reader.getTimes(), [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5])

        # Time
        reader.setParams(timestep=None, time=1.01)
        tdata, tdata1 = reader.getCurrentTimeInformation()
        self.assertIsNone(tdata1)
        self.assertAlmostEqual(tdata.time, 1)
        self.assertEqual(tdata.timestep, 2)
        self.assertEqual(tdata.index, 0)
        self.assertEqual(tdata.filename, self.multiple + '-s002')

        # Wait and then "update" the first few files
        time.sleep(1.5)
        for i in range(6):
            mooseutils.touch(self.testfiles[i])

        reader.setParams(time=None, timestep=-1)
        tdata, tdata1 = reader.getCurrentTimeInformation()
        self.assertIsNone(tdata1)
        self.assertEqual(reader.getTimes(), [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0])
        self.assertAlmostEqual(tdata.time, 3.0)
        self.assertEqual(tdata.timestep, 6)
        self.assertEqual(tdata.index, 0)
        self.assertEqual(tdata.filename, self.multiple + '-s006')

    def testReload(self):
        """
        Test the file reloading is working.
        """
        filenames = ['../input/diffusion_1.e', '../input/diffusion_2.e']
        common = 'common.e'
        shutil.copy(filenames[0], common)
        reader = chigger.exodus.ExodusReader(common)
        self.assertEqual(reader.getTimes(), [0.0, 0.1])

        shutil.copy(filenames[1], common)
        self.assertEqual(reader.getTimes(), [0.0, 0.1, 0.2])

        shutil.copy(filenames[0], common)
        self.assertEqual(reader.getTimes(), [0.0, 0.1])

    def testVariableReload(self):
        filenames = ['../input/simple_diffusion_out.e', '../input/simple_diffusion_new_var_out.e']
        common = 'common.e'
        shutil.copy(filenames[0], common)
        reader = chigger.exodus.ExodusReader(common)
        variables = reader.getVariableInformation()
        self.assertIn('aux', [v.name for v in variables[chigger.exodus.ExodusReader.ELEMENTAL]])
        self.assertIn('u', [v.name for v in variables[chigger.exodus.ExodusReader.NODAL]])
        self.assertNotIn('New_0', [v.name for v in variables[chigger.exodus.ExodusReader.NODAL]])

        time.sleep(1.5) # make sure modified time is different (MacOS requires > 1s)
        shutil.copy(filenames[1], common)
        variables = reader.getVariableInformation()
        self.assertIn('aux', [v.name for v in variables[chigger.exodus.ExodusReader.ELEMENTAL]])
        self.assertIn('u', [v.name for v in variables[chigger.exodus.ExodusReader.NODAL]])
        self.assertIn('New_0', [v.name for v in variables[chigger.exodus.ExodusReader.NODAL]])

    def testVariableNameDuplicate(self):

        # Basic read
        reader = chigger.exodus.ExodusReader(self.duplicate)
        variables = reader.getVariableInformation()
        self.assertEqual(len(variables), 3)

        e_vars = variables[chigger.exodus.ExodusReader.ELEMENTAL]
        self.assertEqual(e_vars[0].name, 'variable')
        self.assertEqual(e_vars[0].fullname, 'variable::ELEMENTAL')
        self.assertEqual(e_vars[0].num_components, 1)
        self.assertEqual(e_vars[0].object_type, chigger.exodus.ExodusReader.ELEMENTAL)
        self.assertTrue(e_vars[0].active)

        g_vars = variables[chigger.exodus.ExodusReader.GLOBAL]
        self.assertEqual(g_vars[0].name, 'variable')
        self.assertEqual(g_vars[0].fullname, 'variable::GLOBAL')
        self.assertEqual(g_vars[0].num_components, 1)
        self.assertEqual(g_vars[0].object_type, chigger.exodus.ExodusReader.GLOBAL)
        self.assertTrue(g_vars[0].active)

        n_vars = variables[chigger.exodus.ExodusReader.NODAL]
        self.assertEqual(n_vars[0].name, 'variable')
        self.assertEqual(n_vars[0].fullname, 'variable::NODAL')
        self.assertEqual(n_vars[0].num_components, 1)
        self.assertEqual(n_vars[0].object_type, chigger.exodus.ExodusReader.NODAL)
        self.assertTrue(n_vars[0].active)

        # Check for warning if just 'variables' is provided
        with self.assertLogs() as l:
            reader.setParams(variables=('variable',))
            variables = reader.getVariableInformation()
        self.assertIn("The variable name 'variable' exists with", l.output[0])

        # Check active state with suffix supplied
        reader.setParams(variables=('variable::NODAL',))
        variables = reader.getVariableInformation()
        self.assertFalse(variables[chigger.exodus.ExodusReader.ELEMENTAL][0].active)
        self.assertFalse(variables[chigger.exodus.ExodusReader.GLOBAL][0].active)
        self.assertTrue(variables[chigger.exodus.ExodusReader.NODAL][0].active)

        # Check for error if bad suffix given
        with self.assertLogs() as l:
            reader.setParams(variables=('variable::WRONG',))
            variables = reader.getVariableInformation()
        self.assertIn("Unknown variable prefix '::WRONG'", l.output[0])


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
