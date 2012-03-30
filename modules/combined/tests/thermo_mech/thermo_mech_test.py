from options import *

test = { INPUT : 'thermo_mech_test.i',
         EXODIFF : ['out.e'],
         SCALE_REFINE : 1
         }

testSMP = { INPUT : 'thermo_mech_smp_test.i',
            EXODIFF : ['out_smp.e'],
            CUSTOM_CMP : 'thermo_mech_test.cmp'}

