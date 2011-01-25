import tools

def bad_kernel_test():
  tools.executeAppExpectError(__file__,'bad_kernel_test.i',"A _Foo_ is not a registered Kernel")

def bad_bc_test():
  tools.executeAppExpectError(__file__,'bad_bc_test.i',"A _Foo_ is not a registered BC")

def bad_material_test():
  tools.executeAppExpectError(__file__,'bad_material_test.i',"A _Foo_ is not a registered Material")

def bad_executioner_test():
  tools.executeAppExpectError(__file__,'bad_executioner_test.i',"A _Foo_ is not a registered Executioner")

def no_output_dir_test():
  tools.executeAppExpectError(__file__,'bad_output_dir_test.i',"Can not write to directory: ./bad_dir for file base: bad_dir/out")

def missing_mesh_test():
  tools.executeAppExpectError(__file__,'missing_mesh_test.i',"connot locate specified file:")

def bad_material_block_test():
  tools.executeAppExpectError(__file__,'bad_material_block_test.i','Material block "0" specified in the input file does not exist')

def bad_kernel_var_test():
  tools.executeAppExpectError(__file__,'bad_kernel_var_test.i','variable foo does not exist in this system')

def bad_bc_var_test():
  tools.executeAppExpectError(__file__,'bad_bc_var_test.i','variable foo does not exist in this system')

def incomplete_kernel_coverage_test():
  pass
#  tools.executeAppExpectError(__file__,'incomplete_kernel_coverage_test.i','variable foo does not exist in this system')
