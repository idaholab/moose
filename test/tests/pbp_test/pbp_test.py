from options import *

test = { INPUT : 'pbp_test.i',
         EXODIFF : ['out.e'] }

check_petsc_options_test = { INPUT : 'pbp_test.i',
                             EXPECT_ERR : 'KSP Residual'}
