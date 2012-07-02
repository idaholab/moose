from options import *

testie = { INPUT : 'ie.i',
           EXODIFF : ['out_ie.e'] }

testbdf2 = { INPUT : 'bdf2.i',
             EXODIFF : ['out_bdf2.e'] }

testcranic = { INPUT : 'cranic.i',
               EXODIFF : ['out_cranic.e'] }

testdt2 = { INPUT : 'dt2.i',
            EXODIFF : ['out_dt2.e'] }

# versions with adaptivity

test_ie_adapt = { INPUT : 'ie_adapt.i',
                  EXODIFF : ['out_ie_adapt.e-s005'],
                  GROUP : 'adaptive'}

test_bdf2_adapt = { INPUT : 'bdf2_adapt.i',
                    EXODIFF : ['out_bdf2_adapt.e-s005'],
                    GROUP : 'adaptive'}

test_cranic_adapt = { INPUT : 'cranic_adapt.i',
                      EXODIFF : ['out_cranic_adapt.e-s005'],
                      GROUP : 'adaptive',
                      SKIP : 'wrong' }

testdt2 = { INPUT : 'dt2_adapt.i',
            EXODIFF : ['out_dt2_adapt.e-s038'],
            GROUP : 'adaptive'}

# The test varies the time step based on wall clock solve time
# which is not consistent for testing - we will check only that 
# the output file exists
test_solution_time_adaptive = { INPUT : 'time-adaptive.i',
                                CHECK_FILES : ['out_time_adaptive.e'] }
