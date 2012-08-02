from options import *

smp_test = {
  INPUT : 'smp_single_test.i',
  EXODIFF : ['smp_single_test_out.e']
}

smp_adapt_test = {
  INPUT : 'smp_single_adapt_test.i',
  EXODIFF : ['smp_single_adapt_test_out.e-s004'],
  MAX_PARALLEL : 1,
  GROUP : 'adaptive'
}

