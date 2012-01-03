from options import *

test = { INPUT : 'heat_conduction_patch_test.i',
         EXODIFF : ['out.e']}

test_hex20 = { INPUT : 'heat_conduction_patch_hex20_test.i',
               EXODIFF : ['out_hex20.e']}

test_rz = { INPUT : 'heat_conduction_patch_rz_test.i',
            EXODIFF : ['out_rz.e']}

test_rz_quad8 = { INPUT : 'heat_conduction_patch_rz_quad8_test.i',
                  EXODIFF : ['out_rz_quad8.e']}

