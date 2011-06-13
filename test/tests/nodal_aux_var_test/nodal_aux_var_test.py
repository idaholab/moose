from options import *

test = { INPUT : 'nodal_aux_var_test.i',
         EXODIFF : ['out.e'] }

ts_test = { INPUT : 'nodal_aux_ts_test.i',
            EXODIFF : ['out_ts.e'],
            SKIP : True }

multi_update_test = { INPUT : 'multi_update_var_test.i',
                      EXODIFF : ['out_multi_var.e'] }

