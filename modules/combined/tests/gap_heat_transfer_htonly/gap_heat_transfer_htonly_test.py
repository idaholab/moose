from options import *

test = { INPUT : 'gap_heat_transfer_htonly_test.i',
         EXODIFF : ['gap_heat_transfer_htonly_test_out.e'],
         ABS_ZERO : 1e-6 }

test_it = { INPUT : 'gap_heat_transfer_htonly_it_plot_test.i',
            EXODIFF : ['out_it_plot.e'],
            CUSTOM_CMP : 'gap_heat_transfer_htonly_it_plot_test.cmp'}

test_rz = { INPUT : 'gap_heat_transfer_htonly_rz_test.i',
            EXODIFF : ['out_rz.e'],
            ABS_ZERO : 1e-9,
            REL_ERR : 5e-3 }
