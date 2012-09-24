from options import *

bad_kernel_test = { INPUT : 'bad_kernel_test.i',
                    EXPECT_ERR : "A '\w+' is not a registered object" }

bad_bc_test = { INPUT : 'bad_bc_test.i',
                EXPECT_ERR : "A '\w+' is not a registered object" }

bad_material_test = { INPUT : 'bad_material_test.i',
                      EXPECT_ERR : "A '\w+' is not a registered object" }

nodal_material_test = { INPUT : 'nodal_material_test.i',
                        EXPECT_ERR : "Nodal AuxKernel '\w+' attempted to reference material property '\w+'" }

bad_executioner_test = { INPUT : 'bad_executioner_test.i',
                         EXPECT_ERR : "A '\w+' is not a registered object" }

no_output_dir_test = { INPUT : 'no_output_dir_test.i',
                       EXPECT_ERR : "Can not write to directory: \S+ for file base: \S+" }

missing_mesh_test = { INPUT : 'missing_mesh_test.i',
                      EXPECT_ERR : "Unable to open file \S+" }

missing_function_test = { INPUT : 'missing_function_test.i',
                          EXPECT_ERR : "Unable to find function \S+" }

missing_function_file_test = { INPUT : 'missing_function_file_test.i',
                               EXPECT_ERR : "Error opening file \S+ from PiecewiseLinearFile function" }

function_file_test1 = { INPUT : 'function_file_test1.i',
                        EXPECT_ERR : "Read more than two rows of data from file '\S+' for PiecewiseLinearFile function.  Did you mean to use \"format = columns\"?" }

function_file_test2 = { INPUT : 'function_file_test2.i',
                        EXPECT_ERR : "Read more than 2 columns of data from file '\S+' for PiecewiseLinearFile function.  Did you mean to use \"format = rows\"?" }

function_file_test3 = { INPUT : 'function_file_test3.i',
                        EXPECT_ERR : "Lengths of x and y data do not match in file '\S+' for PiecewiseLinearFile function." }

function_file_test4 = { INPUT : 'function_file_test4.i',
                        EXPECT_ERR : "Invalid option for format: \S+ in PiecewiseLinearFile.  Valid options are rows and columns." }

bad_material_block_test = { INPUT : 'bad_material_block_test.i',
                            EXPECT_ERR : 'Material block \S+ specified in the input file does not exist' }

missing_material_prop_test = { INPUT : 'missing_material_prop_test.i',
                               EXPECT_ERR : "No material defined on block \d+" }

missing_material_prop_test2 = { INPUT : 'missing_material_prop_test2.i',
                               EXPECT_ERR : "Material property 'diff1' is not defined on block \d+" }

missing_coupled_mat_prop_test = { INPUT : 'missing_coupled_mat_prop_test.i',
                                  EXPECT_ERR : 'One or more Material Properties were not supplied on block' }

bad_kernel_var_test = { INPUT : 'bad_kernel_var_test.i',
                        EXPECT_ERR : 'Unknown variable' }

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

missing_req_par_mesh_block_test = { INPUT : 'missing_req_par_mesh_block_test.i',
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

exception_test = {
  INPUT : 'exception_test.i',
  PETSC_VERSION : ['3.0.0'],
  EXPECT_ERR : "Exception caught: MOOSE exception"
}

multi_precond_test = { INPUT : 'multi_precond_test.i',
                       EXPECT_ERR : 'More than one active Preconditioner Action detected while building \S+' }

bad_second_order_test = { INPUT : 'bad_second_order_test.i',
                          EXPECT_ERR : 'Error in libMesh internal logic',
                          SHOULD_CRASH : True,
                          SKIP : 'Inconsistent Error'}

wrong_object_test = { INPUT : 'wrong_moose_object_test.i',
                      EXPECT_ERR : 'Inconsistent Action Name detected!' }

wrong_object_test2 = { INPUT : 'nonexistent_pps_test.i',
                       EXPECT_ERR : "Postprocessor '\S+' requested but not specified in the input file." }

subdomain_restricted_auxkernel_test = { INPUT : 'subdomain_restricted_auxkernel_mismatch.i',
                                        EXPECT_ERR : "AuxKernel \(\w+\): block outside of the domain of the variable" }

subdomain_restricted_kernel_test = { INPUT : 'subdomain_restricted_kernel_mismatch.i',
                                     EXPECT_ERR : "Kernel \(\w+\): block outside of the domain of the variable" }

bad_enum_test = { INPUT : 'bad_enum_test.i',
                  EXPECT_ERR : "Invalid option \"\w+\" in MooseEnum." }

deprecated_block_test = {
  INPUT : 'deprecated_block_test.i',
  EXPECT_ERR : "Input file block '\S+' has been deprecated."
}

unused_param_test = {
 INPUT : 'unused_param_test.i',
 CLI_ARGS : ['--warn-unused'],
 EXPECT_OUT : 'WARNING: The following parameters were unused in your input file'
}

coupled_dot_aux_var_test = {
  INPUT : 'coupled_dot_aux_var_test.i',
  EXPECT_ERR : 'Coupling time derivative of an auxiliary variable is not allowed.'
}

invalid_elemental_to_nodal_couple_test = {
    INPUT : 'invalid_aux_coupling_test.i',
    EXPECT_ERR : "You cannot couple an elemental variable to a nodal variable"
}

same_name_variable_test = {
  INPUT : 'same_name_variable_test.i',
  EXPECT_ERR : "Cannot have an auxiliary variable and a nonlinear variable with the same name!"
}

override_name_variable_test = {
  INPUT : 'override_name_variable_test.i',
  EXPECT_OUT : "The following variables were overridden or supplied multiple times:",
}

pps_bad_block_test = { INPUT : 'pps_bad_block_test.i',
                       EXPECT_ERR : "One or more UserObjects is referencing a nonexistent block" }

dynamic_check_name_block_test = { INPUT : 'check_dynamic_name_block.i',
                                  EXPECT_ERR : "The following dynamic block name is not unique: \w+" }

dynamic_check_name_boundary_test = { INPUT : 'check_dynamic_name_boundary.i',
                                     EXPECT_ERR : "The following dynamic boundary name is not unique: \w+" }

dynamic_check_name_block_mismatch_test = { INPUT : 'check_dynamic_name_block_mismatch.i',
                                           EXPECT_ERR : "You must supply the same number of block ids and names parameters" }

dynamic_check_name_boundary_mismatch_test = { INPUT : 'check_dynamic_name_boundary_mismatch.i',
                                              EXPECT_ERR : "You must supply the same number of boundary ids and names parameters" }
add_aux_variable_multiple_test = { INPUT : 'add_aux_variable_multiple_test.i',
                                   EXPECT_ERR : "AuxVariable with name 'q' already exists but is of a differing type!" }
double_restrict_uo_test = { INPUT : 'double_restrict_uo.i',
                            EXPECT_ERR : "The parameter 'boundary' and 'block' were both supplied for \w+" }
uo_pps_name_collision_test = { INPUT : 'uo_pps_name_collision_test.i',
                               EXPECT_ERR : 'A UserObject with the name "\w+" already exists' }
