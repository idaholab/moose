#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Input import OutputNames, InputTree, ExecutableInfo
from peacock.utils import Testing
import datetime
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def create_tree(self, input_file):
        app_info = ExecutableInfo.ExecutableInfo()
        app_info.setPath(Testing.find_moose_test_exe())
        self.assertTrue(app_info.valid())
        input_tree = InputTree.InputTree(app_info)
        input_tree.setInputFile(input_file)
        return input_tree

    def testOutputFiles(self):
        input_file = "../../common/transient.i"
        input_tree = self.create_tree(input_file)
        output_names = OutputNames.getOutputFiles(input_tree, input_file)
        self.assertEqual(output_names, ["out_transient.e"])
        outputs = input_tree.getBlockInfo("/Outputs")
        file_base = outputs.getParamInfo("file_base")
        file_base.value = "new_file_base"
        outputs.parameters_list.remove("file_base")
        del outputs.parameters["file_base"]
        output_names = OutputNames.getOutputFiles(input_tree, input_file)
        self.assertEqual(output_names, ["transient_out.e"])

    def testOversample(self):
        input_file = "../../common/oversample.i"
        input_tree = self.create_tree(input_file)
        output_names = OutputNames.getOutputFiles(input_tree, input_file)
        self.assertEqual(output_names, ["out_transient.e", "oversample_2.e"])

        outputs = input_tree.getBlockInfo("/Outputs")
        outputs.parameters_list.remove("file_base")
        del outputs.parameters["file_base"]

        output_names = OutputNames.getOutputFiles(input_tree, input_file)
        self.assertEqual(output_names, ["oversample_out.e", "oversample_2.e"])

        outputs = input_tree.getBlockInfo("/Outputs/refine_2")
        t = outputs.getTypeBlock()
        t.parameters_list.remove("file_base")
        del t.parameters["file_base"]

        output_names = OutputNames.getOutputFiles(input_tree, input_file)
        self.assertEqual(output_names, ["oversample_out.e", "oversample_refine_2.e"])

    def testDate(self):
        input_file = "../../common/transient_with_date.i"
        input_tree = self.create_tree(input_file)
        output_names = OutputNames.getOutputFiles(input_tree, input_file)
        utc = datetime.datetime.utcnow()
        self.assertEqual(output_names, ["with_date.e", "with_date_%s.e" % utc.strftime("%Y-%m-%d")])

    def testPostprocessor(self):
        input_file = "../../common/transient.i"
        input_tree = self.create_tree(input_file)
        output_names = OutputNames.getPostprocessorFiles(input_tree, input_file)
        self.assertEqual(output_names, ["out_transient.csv"])

        outputs = input_tree.getBlockInfo("/Outputs")
        outputs.parameters_list.remove("file_base")
        del outputs.parameters["file_base"]

        output_names = OutputNames.getPostprocessorFiles(input_tree, input_file)
        self.assertEqual(output_names, ["transient_out.csv"])

    def testVectorPostprocessor(self):
        input_file = "../../common/time_data.i"
        input_tree = self.create_tree(input_file)
        output_names = OutputNames.getVectorPostprocessorFiles(input_tree, input_file)
        self.assertEqual(output_names, ["time_data_line_sample_*.csv"])

        outputs = input_tree.getBlockInfo("/Outputs")
        p = outputs.getParamInfo("file_base")
        p.value = "foo"

        output_names = OutputNames.getVectorPostprocessorFiles(input_tree, input_file)
        self.assertEqual(output_names, ["foo_line_sample_*.csv"])

        outputs.parameters_list.remove("file_base")
        del outputs.parameters["file_base"]

        output_names = OutputNames.getVectorPostprocessorFiles(input_tree, input_file)
        self.assertEqual(output_names, ["time_data_out_line_sample_*.csv"])

if __name__ == '__main__':
    Testing.run_tests()
