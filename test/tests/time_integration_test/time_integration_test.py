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
                  EXODIFF : ['out_ie_adapt_0004.e'] }

test_bdf2_adapt = { INPUT : 'bdf2_adapt.i',
                    EXODIFF : ['out_bdf2_adapt_0004.e'] }

test_cranic_adapt = { INPUT : 'cranic_adapt.i',
                      EXODIFF : ['out_cranic_adapt_0004.e'] }

testdt2 = { INPUT : 'dt2_adapt.i',
            EXODIFF : ['out_dt2_adapt_0037.e'] }

test_solution_time_adaptive = { INPUT : 'time-adaptive.i',
                                EXODIFF : ['out_time_adaptive.e'],
                                SKIP : True }
