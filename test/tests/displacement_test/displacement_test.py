from options import *

test = { INPUT : 'displacement_test.i',
         EXODIFF : ['out_displaced.e-s002'],
         GROUP : 'adaptive'}

displacement_transient_test = {
  INPUT : 'displacement_transient_test.i',
  EXODIFF : ['displacement_transient_test_out_displaced.e-s011'],
  GROUP : 'adaptive'}
