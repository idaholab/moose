#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Input.ExecutableInfo import ExecutableInfo
from peacock.Input.InputTree import InputTree
import os
from peacock.utils import Testing
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.transient = "../../common/transient.i"
        base = os.path.dirname(os.path.realpath(__file__))
        self.transient_gold = os.path.join(base, "gold/transient.i")
        self.fsp = "../../common/fsp_test.i"
        self.fsp_gold = os.path.join(base, "gold/fsp_test.i")
        self.simple_diffusion = "../../common/simple_diffusion.i"
        self.simple_diffusion_gold = os.path.join(base, "gold/simple_diffusion.i")
        self.lcf1 = "../../common/lcf1.i"
        self.lcf1_gold = os.path.join(base, "gold/lcf1.i")

    def createExeInfo(self):
        e = ExecutableInfo()
        e.setPath(Testing.find_moose_test_exe(dirname="modules/combined", exe_base="combined"))
        return e

    def createTree(self, valid=True, input_file=None):
        if valid:
            e = self.createExeInfo()
        else:
            e = ExecutableInfo()
        t = InputTree(e)
        if input_file:
            t.setInputFile(input_file)
        return t

    def testInputFileOnly(self):
        t = self.createTree(valid=False, input_file="")
        b = t.getBlockInfo("/Mesh")
        self.assertEqual(b, None)
        t.setInputFile(self.transient)
        self.assertNotEqual(t.input_file, None)
        self.assertEqual(t.input_filename, os.path.abspath(self.transient))
        b = t.getBlockInfo("/Mesh")
        self.assertEqual(b, None)

    def testTransient(self):
        t = self.createTree(input_file=self.transient)
        self.checkFile(t.getInputFileString(), self.transient_gold, True)

    def testLCF(self):
        t = self.createTree(input_file=self.lcf1)
        self.checkFile(t.getInputFileString(), self.lcf1_gold, True)

    def testFSP(self):
        t = self.createTree(input_file=self.fsp)
        self.checkFile(t.getInputFileString(), self.fsp_gold, True)

    def testSimpleDiffusion(self):
        t = self.createTree(input_file=self.simple_diffusion)
        self.checkFile(t.getInputFileString(), self.simple_diffusion_gold, True)

    def testChangingInputFiles(self):
        t = self.createTree(input_file=self.simple_diffusion)
        self.checkFile(t.getInputFileString(), self.simple_diffusion_gold)

        t.setInputFile(self.lcf1)
        self.checkFile(t.getInputFileString(), self.lcf1_gold)

        t.setInputFile(self.fsp)
        self.checkFile(t.getInputFileString(), self.fsp_gold)

        t.setInputFile(self.transient)
        self.checkFile(t.getInputFileString(), self.transient_gold)

    def testTreeWithOnlyApp(self):
        t = self.createTree()
        p = t.path_map.get("/Preconditioning")
        self.assertNotEqual(p, None)
        self.assertEqual(p.star, True)


    def testAddVectorPostprocessor(self):
        """
        Add a new VectorPostprocessor to simple diffusion
        """
        t = self.createTree(input_file=self.simple_diffusion)
        v = t.getBlockInfo("/VectorPostprocessors")
        v.included = True
        b = t.addUserBlock("/VectorPostprocessors", "foo")
        b.included = True
        self.assertNotEqual(b, None)
        b.setParamValue("type", "LineValueSampler")
        b.setParamValue("num_points", "10")
        b.setParamValue("start_point", "0 0 0")
        b.setParamValue("end_point", "1 0 0")

        #self.checkFile(t.getInputFileString(), "gold/simple_diffusion_vp.i", True)

    def testBlocks(self):
        t = self.createTree(input_file=self.simple_diffusion)
        c = t.cloneUserBlock("/NoExist", "foo")
        self.assertEqual(c, None)

        b = t.getBlockInfo("/BCs/left")
        self.assertNotEqual(b, None)
        self.assertEqual(b.user_added, True)
        p = b.addUserParam("foo", "bar")

        c = t.cloneUserBlock(b.path, "left1")
        self.assertNotEqual(c, None)
        self.assertEqual(c.name, "left1")
        self.assertEqual(b.name, "left")
        self.assertEqual(c.paramValue("foo"), "bar")
        self.assertIn(c.path, t.path_map)

        self.assertEqual(c.parent.children_list.index(c.name), 7)
        t.moveBlock("/NoExist", 0)
        t.moveBlock(c.path, 0)
        self.assertEqual(c.parent.children_list.index(c.name), 0)

        # make sure they aren't sharing a parameter
        c.setParamValue("foo", "bar1")
        self.assertEqual(c.paramValue("foo"), "bar1")
        self.assertEqual(p.value, "bar")

        t.renameUserBlock("/NoExist", "foo", "foo1")

        t.renameUserBlock(b.parent.path, b.name, "foo1")
        self.assertEqual(b.path, "/BCs/foo1")
        self.assertEqual(b.name, "foo1")
        self.assertIn(b.name, b.parent.children)
        self.assertIn(b.name, b.parent.children_list)
        self.assertIn(b.path, t.path_map)

        self.assertEqual(b.included, True)
        t.setBlockSelected(b.path, False)
        self.assertEqual(b.included, False)

        t.setBlockSelected(b.path, False)
        self.assertEqual(b.included, False)

        t.setBlockType("/NoExist", "foo")
        t.setBlockType(b.path, "foo")

        b = t.addUserBlock("/", "foo")
        self.assertEqual(b, None)

        b = t.addUserBlock("/BCs", "foo")
        self.assertNotEqual(b, None)
        self.assertEqual(b.path, "/BCs/foo")
        self.assertEqual(b.user_added, True)

        p = b.parent
        t.removeUserBlock(p.path, b.name)
        self.assertEqual(t.getBlockInfo(b.path), None)
        self.assertNotIn(b.name, p.children_list)
        self.assertNotIn(b.name, p.children)

    def testIncompatible(self):
        t = InputTree(ExecutableInfo())
        app_info = ExecutableInfo()
        # The original tree wasn't set, any new changes
        # are not incompatible
        self.assertFalse(t.incompatibleChanges(app_info))
        app_info.json_data = {"bad_block": None}
        self.assertFalse(t.incompatibleChanges(app_info))

        # Current tree is good, new one is not
        app_info.json_data = None
        t = self.createTree(input_file=self.simple_diffusion)
        self.assertTrue(t.incompatibleChanges(app_info))
        app_info.json_data = {"bad_block": None}
        self.assertFalse(app_info.valid())
        self.assertTrue(t.incompatibleChanges(app_info))
        app_info.path ="foo"
        self.assertTrue(app_info.valid())
        self.assertTrue(t.incompatibleChanges(app_info)) # No / which causes an exception
        app_info.path_map = t.app_info.path_map.copy()
        self.assertFalse(t.incompatibleChanges(app_info)) # Should be the same, no problems

        # Simulate removing /BCs/*/<types>/DirichletBC/boundary
        # This should be fine since users can have extra parameters
        # in their input file
        root = app_info.path_map["/"]
        bcs = root.children["BCs"]
        pname = "boundary"
        tmp = bcs.star_node.types["DirichletBC"]
        p = tmp.parameters[pname]
        p.user_added = True
        tmp.removeUserParam(pname)
        p = bcs.star_node.parameters[pname]
        p.user_added = True
        bcs.star_node.removeUserParam(pname)
        tmp_tree = InputTree(app_info)

        tmp_tree.setInputFileData(t.getInputFileString())
        left = tmp_tree.getBlockInfo("/BCs/left")
        self.assertTrue(left.user_added)
        boundary = left.parameters["boundary"]
        self.assertTrue(boundary.user_added)
        self.assertTrue(boundary.set_in_input_file)
        self.assertFalse(t.incompatibleChanges(app_info))

        # Simulate removing /BCs syntax
        # This should cause a problem
        del app_info.path_map["/BCs"]
        root.children_list.remove("BCs")
        del root.children["BCs"]
        self.assertTrue(t.incompatibleChanges(app_info)) # Errors when reading the input file

if __name__ == '__main__':
    Testing.run_tests()
