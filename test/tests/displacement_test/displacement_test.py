from options import *

test = { INPUT : 'displacement_test.i',
         EXODIFF : ['out_displaced.e-s002'],
         GROUP : 'adaptive'}

displacement_transient_test = {
  INPUT : 'displacement_transient_test.i',
  EXODIFF : ['displacement_transient_test_out_displaced.e-s011'],
  MAX_PARALLEL : 1,
  GROUP : 'adaptive'}

displaced_eq_test = {
  INPUT : 'displaced_eq_transient_test.i',
  EXODIFF : ['displaced_eq_transient_test_out_displaced.e-s011'],
  MAX_PARALLEL : 1,
  GROUP : 'adaptive'}
