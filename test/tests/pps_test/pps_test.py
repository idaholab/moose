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
                   EXODIFF : ['out_nodal_max.e'],
                   MAX_PARALLEL : 11
                   }

test_elem_multi_block = {
  INPUT : 'elem_pps_multi_block_test.i',
  EXODIFF : ['elem_pps_multi_block_test_out.e']
}

test_side_multi_bnd = {
  INPUT : 'side_pps_multi_bnd_test.i',
  EXODIFF : ['side_pps_multi_bnd_test_out.e']
}

# The PPS tables should have only 2 rows of real data not counting the header or continuation line
# This RegEx matches the continuation line and exactly 2 lines of output followed by the table closing line
screen_output_test = { INPUT : 'screen_output_test.i',
                       EXPECT_OUT : '^:\s+:.*\n(?:^\|\s+\d\.\d.*\n){2}^\+(-+\+)+\n' }

pps_output_test = {
  INPUT : 'pps_output_test.i',
  EXODIFF : ['pps_output_test_out.e']
}

pps_old_test = {
  INPUT : 'pps_old_value.i',
  EXODIFF : ['pps_old_value_out.e']
}
