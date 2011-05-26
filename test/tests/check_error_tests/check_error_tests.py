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

def missing_active_section_test():
  tools.executeAppExpectError(__file__,'missing_active_section.i',"One or more active lists in the input file are missing a referenced section")

def invalid_steady_exec_test():
  tools.executeAppExpectError(__file__,'invalid_steady_exec_test.i','You have specified time kernels in your steady state simulation')

def stateful_adaptive_error_test():
  tools.executeAppExpectError(__file__,'stateful_adaptive_error_test.i','Cannot use Material classes with stateful properties while utilizing adaptivity')

def windows_line_endings_test():
  tools.executeAppExpectError(__file__,'windows_line_endings.i',"\S+ contains Windows\(DOS\) line endings")

def nan_test():
  tools.executeAppExpectError(__file__,'nan_test.i',"Floating point exception")
  
try:
  from options import *

  bad_kernel_test = { INPUT : 'bad_kernel_test.i',
                      EXPECT_ERR : "A '\w+' is not a registered object" }

  bad_bc_test = { INPUT : 'bad_bc_test.i',
                  EXPECT_ERR : "A '\w+' is not a registered object" }

  bad_material_test = { INPUT : 'bad_material_test.i',
                        EXPECT_ERR : "A '\w+' is not a registered object" }

  bad_executioner_test = { INPUT : 'bad_executioner_test.i',
                           EXPECT_ERR : "A '\w+' is not a registered object" }

  bad_executioner_test = { INPUT : 'bad_executioner_test.i',
                           EXPECT_ERR : "A '\w+' is not a registered object" }

  no_output_dir_test = { INPUT : 'no_output_dir_test.i',
                         EXPECT_ERR : "Can not write to directory: \S+ for file base: \S+" }

  missing_mesh_test = { INPUT : 'missing_mesh_test.i',
                        EXPECT_ERR : "Unable to open file \S+" }

  bad_material_block_test = { INPUT : 'bad_material_block_test.i',
                              EXPECT_ERR : 'Material block \S+ specified in the input file does not exist' }

  missing_material_test = { INPUT : 'missing_material_test.i',
                            EXPECT_ERR : "The following blocks from your input mesh do not contain on active material: \d+" }
  
  bad_kernel_var_test = { INPUT : 'bad_kernel_var_test.i',
                          EXPECT_ERR : 'variable foo does not exist in this system' }

  bad_bc_var_test = { INPUT : 'bad_bc_var_test.i',
                      EXPECT_ERR : 'variable foo does not exist in this system' }

  incomplete_kernel_block_coverage_test = { INPUT : 'incomplete_kernel_block_coverage_test.i',
                                            EXPECT_ERR : 'The following block\(s\) lack an active kernel:' }

  missing_mesh_bcs_test = { INPUT : 'missing_mesh_bcs_test.i',
                            EXPECT_ERR : 'The following boundary ids from your input file do not exist in the input mesh \d+' }

  missing_req_par_moose_obj_test = { INPUT : 'missing_req_par_moose_obj_test.i',
                                     EXPECT_ERR : "The required parameter '\S+' is missing" }

  missing_req_par_action_obj_test = { INPUT : 'missing_req_par_action_obj_test.i',
                                      EXPECT_ERR : "The required parameter '\S+' is missing" }

  missing_active_section_test = { INPUT : 'missing_active_section.i',
                                  EXPECT_ERR : "One or more active lists in the input file are missing a referenced section" }

  invalid_steady_exec_test = { INPUT : 'invalid_steady_exec_test.i',
                               EXPECT_ERR : 'You have specified time kernels in your steady state simulation' }

  stateful_adaptive_error_test = { INPUT : 'stateful_adaptive_error_test.i',
                                   EXPECT_ERR : 'Cannot use Material classes with stateful properties while utilizing adaptivity' }

  windows_line_endings_test = { INPUT : 'windows_line_endings.i',
                                EXPECT_ERR : "\S+ contains Windows\(DOS\) line endings" }

  nan_test = { INPUT : 'nan_test.i',
               EXPECT_ERR : "Floating point exception" }

except:
  pass
