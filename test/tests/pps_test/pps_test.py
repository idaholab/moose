from options import *

test_nodal_var_value = { INPUT : 'nodal_var_value.i',
                         EXODIFF : ['out_nodal_var_value.e'] }

test_nodal_aux_var_value = { INPUT : 'nodal_aux_var_value.i',
                             EXODIFF : ['out_nodal_aux_var_value.e'] }

test_avg_nodal_var_value = { INPUT : 'avg_nodal_var_value.i',
                             EXODIFF : ['out_avg_nodal_var_value.e'] }

test_inital = { INPUT : 'initial_pps.i',
                EXODIFF : ['out_initial_pps.e'] }

