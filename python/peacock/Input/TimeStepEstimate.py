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
import mooseutils

def estimateTimeSteps(executioner_node):
    """
    Gets the approximate number of steps from the Exectioner section of the input file
    Inputs:
        executioner_node[BlockInfo]: /Executioner
    """
    cur_steps = 0
    try:
        if executioner_node:
            num_steps = executioner_node.getParamInfo("num_steps")
            if num_steps and num_steps.value:
                cur_steps = int(num_steps.value)
            end_time = executioner_node.getParamInfo("end_time")
            dt = executioner_node.getParamInfo("dt")
            if end_time and end_time.value and dt and float(dt.value) > 0:
                steps = float(end_time.value) / float(dt.value)
                if cur_steps == 0 or steps < cur_steps:
                    cur_steps = steps
            adaptivity = executioner_node.children.get("Adaptivity")
            if adaptivity and adaptivity.included:
                steps = adaptivity.getParamInfo("steps")
                if steps and steps.value:
                    cur_steps += int(steps.value)

        return cur_steps + 2 + 1 # The +2 is for setup steps the +1 is so there is always a bit left...
    except Exception as e:
        mooseutils.mooseWarning("Problem calculating time steps: %s" % e)
        return 0

def findTimeSteps(tree):
    node = tree.getBlockInfo("/Executioner")
    if not node or not node.included:
        return 0
    return estimateTimeSteps(node)

if __name__ == "__main__":
    from peacock.utils import Testing
    from InputTree import InputTree
    from ExecutableInfo import ExecutableInfo
    exe = Testing.find_moose_test_exe()
    this_dir = os.path.dirname(os.path.abspath(__file__))
    peacock_dir = os.path.dirname(this_dir)
    test_file = os.path.join(peacock_dir, "tests", "common", "transient.i")
    exe_info = ExecutableInfo()
    exe_info.path = exe
    tree = InputTree(exe_info)
    tree.inputFileChanged(test_file)
    num_steps = findTimeSteps(tree)
    print("Estimated number of steps: %s" % num_steps)
