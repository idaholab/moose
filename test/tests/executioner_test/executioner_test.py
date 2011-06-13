from options import *

test_steady = { INPUT : 'steady.i',
               EXODIFF : ['out_steady.e'] }

test_steady_adapt = { INPUT : 'steady-adapt.i',
                    EXODIFF : ['out_steady_adapt_0003.e'] }

test_transient = { INPUT : 'transient.i',
                  EXODIFF : ['out_transient.e'] }

test_sln_time_adapt = { INPUT : 'sln-time-adapt.i',
                        EXODIFF : ['out_sta.e'] ,
                        SKIP : True }

