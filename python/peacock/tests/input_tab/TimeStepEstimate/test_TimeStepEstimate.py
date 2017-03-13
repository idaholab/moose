#!/usr/bin/env python
from peacock.Input import TimeStepEstimate
from peacock.Input.ExecutableInfo import ExecutableInfo
from peacock.Input.InputTree import InputTree
from peacock.utils import Testing

class Tests(Testing.PeacockTester):
    def create_tree(self):
        app_info = ExecutableInfo()
        app_info.setPath(Testing.find_moose_test_exe())
        self.assertTrue(app_info.valid())
        input_tree = InputTree(app_info)
        return input_tree

    def testEstimates(self):
        input_tree = self.create_tree()
        input_file = "../../common/transient.i"
        input_tree.setInputFile(input_file)
        num_steps = TimeStepEstimate.findTimeSteps(input_tree)
        self.assertEqual(num_steps, 8)

        b = input_tree.getBlockInfo("/Executioner")
        b.included = False
        self.assertEqual(TimeStepEstimate.findTimeSteps(input_tree), 0)

        b.included = True
        p = b.getParamInfo("num_steps")
        p.value = "foo"
        self.assertEqual(TimeStepEstimate.findTimeSteps(input_tree), 0)

        input_file = "../../common/simple_diffusion.i"
        input_tree.setInputFile(input_file)
        num_steps = TimeStepEstimate.findTimeSteps(input_tree)
        self.assertEqual(num_steps, 3)


if __name__ == '__main__':
    Testing.run_tests()
