import tools

def bad_kernel_test():
  tools.executeAppExpectError(__file__,'bad_kernel_test.i',"A '\w+' is not a registered object")

def bad_bc_test():
  tools.executeAppExpectError(__file__,'bad_bc_test.i',"A '\w+' is not a registered object")

def bad_material_test():
  tools.executeAppExpectError(__file__,'bad_material_test.i',"A '\w+' is not a registered object")

def bad_executioner_test():
  tools.executeAppExpectError(__file__,'bad_executioner_test.i',"A '\w+' is not a registered object")

def no_output_dir_test():
  tools.executeAppExpectError(__file__,'no_output_dir_test.i',"Can not write to directory: \S+ for file base: \S+")

def missing_mesh_test():
  tools.executeAppExpectError(__file__,'missing_mesh_test.i',"Unable to open file \S+")

def bad_material_block_test():
  tools.executeAppExpectError(__file__,'bad_material_block_test.i','Material block \S+ specified in the input file does not exist')

def missing_material_test():
  tools.executeAppExpectError(__file__,'missing_material_test.i',"The following blocks from your input mesh do not contain on active material: \d+")
  
def bad_kernel_var_test():
  tools.executeAppExpectError(__file__,'bad_kernel_var_test.i','variable foo does not exist in this system')

def bad_bc_var_test():
  tools.executeAppExpectError(__file__,'bad_bc_var_test.i','variable foo does not exist in this system')

def incomplete_kernel_block_coverage_test():
  tools.executeAppExpectError(__file__,'incomplete_kernel_block_coverage_test.i','The following block\(s\) lack an active kernel:')

def missing_mesh_bcs_test():
  tools.executeAppExpectError(__file__,'missing_mesh_bcs_test.i','The following boundary ids from your input file do not exist in the input mesh \d+')

def missing_req_par_moose_obj_test():
  tools.executeAppExpectError(__file__,'missing_req_par_moose_obj_test.i',"The required parameter '\S+' is missing")

def missing_req_par_action_obj_test():
  tools.executeAppExpectError(__file__,'missing_req_par_action_obj_test.i',"The required parameter '\S+' is missing")

def invalid_steady_exec_test():
  tools.executeAppExpectError(__file__,'invalid_steady_exec_test.i','You have specified time kernels in your steady state simulation')

def stateful_adaptive_error_test():
  tools.executeAppExpectError(__file__,'stateful_adaptive_error_test.i','Cannot use Material classes with stateful properties while utilizing adaptivity')

def windows_line_endings_test():
  tools.executeAppExpectError(__file__,'windows_line_endings.i',"\S+ contains Windows\(DOS\) line endings")
