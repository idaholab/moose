from options import *

test_steady = { INPUT : 'steady.i',
               EXODIFF : ['out_steady.e'] }

test_steady_adapt = { INPUT : 'steady-adapt.i',
                      EXODIFF : ['out_steady_adapt.e-s004'],
                      GROUP : 'adaptive'}

test_transient = { INPUT : 'transient.i',
                  EXODIFF : ['out_transient.e'] }

test_stead_state_check = {
  INPUT : 'steady_state_check_test.i',
  EXODIFF : ['out_ss_check.e']
}


# This test is invalid (permanently disabled)
# The test varies the time step based on wall clock solve time
# which is not consistent for testing
#test_sln_time_adapt = { INPUT : 'sln-time-adapt.i',
#                        EXODIFF : ['out_sta.e'] ,
#                        SKIP : True }

