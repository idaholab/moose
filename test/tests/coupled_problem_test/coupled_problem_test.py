from options import *

tightly_coupled_test = {
  INPUT : 'tight-coupling.i',
  EXODIFF : ['problem1_out.e', 'problem2_out.e']
}

steady_transient_test = {
  INPUT : 'steady-transient.i',
  EXODIFF : ['steady_out.e', 'transient_out.e']
}
