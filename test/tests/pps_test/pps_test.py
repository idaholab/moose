from options import *

test_nodal_var_value = { INPUT : 'nodal_var_value.i',
                         EXODIFF : ['out_nodal_var_value.e'] }

test_nodal_aux_var_value = { INPUT : 'nodal_aux_var_value.i',
                             EXODIFF : ['out_nodal_aux_var_value.e'] }

test_avg_nodal_var_value = { INPUT : 'avg_nodal_var_value.i',
                             EXODIFF : ['out_avg_nodal_var_value.e'] }

test_avg_nodal_var_value_ts_begin = { INPUT : 'avg_nodal_var_value_ts_begin.i',
                                      EXODIFF : ['out_avg_nodal_var_value_ts_begin.e'] }

test_inital = { INPUT : 'initial_pps.i',
                EXODIFF : ['out_initial_pps.e'] }

test_nodal_max = { INPUT : 'nodal_max_value_test.i',
                   EXODIFF : ['out_nodal_max.e'] }
