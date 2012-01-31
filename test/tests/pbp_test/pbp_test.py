from options import *

test = { INPUT : 'pbp_test.i',
         EXODIFF : ['out.e'] }

check_petsc_options_test = { INPUT : 'pbp_test_options.i',
                             EXPECT_ERR : 'KSP Residual'}

pbp_adapt_test = {
  INPUT : 'pbp_adapt_test.i',
  CUSTOM_CMP : 'pbp_adapt_test.cmp',
  EXODIFF : ['out_pbp_adapt.e-s004'],
  GROUP : 'adaptive'
}

single_test = {
  INPUT : 'pbp_single_test.i',
  EXODIFF : ['pbp_single_test_out.e']
}

single_adapt_test = {
  INPUT : 'pbp_single_adapt_test.i',
  EXODIFF : ['pbp_single_adapt_test_out.e-s004'],
  GROUP : 'adaptive'
}

