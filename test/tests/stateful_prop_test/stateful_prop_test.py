from options import *

test = { INPUT : 'stateful_prop_test.i',
         EXODIFF : ['out.e'],
         MAX_THREADS : 1}

spatial_test = { INPUT : 'stateful_prop_spatial_test.i',
                 EXODIFF : ['out_spatial.e'],
                 MAX_THREADS : 1}

computing_initial_residual_test = { INPUT : 'computing_initial_residual_test.i',
                                    EXODIFF : ['computing_initial_residual_test_out.e'],
                                    MAX_THREADS : 1}
