from options import *

test_penalty_dirichlet_bc = { INPUT : 'penalty_dirichlet_bc_test.i',
                              EXODIFF : ['penalty_dirichlet_bc_test_out.e']
                              }

test_function_penalty_dirichlet_bc = { INPUT : 'function_penalty_dirichlet_bc_test.i',
                                       EXODIFF : ['function_penalty_dirichlet_bc_test_out.e'],
                                       MAX_PARALLEL : 11
                                       }
