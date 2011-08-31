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

missing_material_prop_test = { INPUT : 'missing_material_prop_test.i',
                               EXPECT_ERR : "No material defined on block \d+" }

missing_material_prop_test2 = { INPUT : 'missing_material_prop_test2.i',
                               EXPECT_ERR : "Material property 'diff1' is not defined on block \d+" }

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

multi_precond_test = { INPUT : 'multi_precond_test.i',
                       EXPECT_ERR : 'More than one active Preconditioner Action detected while building \S+' }

bad_second_order_test = { INPUT : 'bad_second_order_test.i',
                          EXPECT_ERR : 'Error in libMesh internal logic' }

wrong_object_test = { INPUT : 'wrong_moose_object_test.i',
                      EXPECT_ERR : 'Inconsistent Action Name detected!' }

assertion_test = { INPUT : 'assertion_test.i',
                   EXPECT_ASSERT : "Assertion `_parser_handle._mesh == NULL' failed" }

wrong_object_test = { INPUT : 'nonexistent_pps_test.i',
                      EXPECT_ERR : "Postprocessor '\S+' requested but not specified in the input file." }
                   