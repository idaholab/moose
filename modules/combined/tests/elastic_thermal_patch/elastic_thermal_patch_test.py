from options import *

test = { INPUT : 'elastic_thermal_patch_test.i',
         EXODIFF : ['out.e'],
         USE_OLD_FLOOR : True,
         ABS_ZERO : 1e-8 }

test_rz = { INPUT : 'elastic_thermal_patch_rz_test.i',
            EXODIFF : ['out_rz.e'],
            USE_OLD_FLOOR : True}

test_rz_smp = { INPUT : 'elastic_thermal_patch_rz_smp_test.i',
                EXODIFF : ['out_rz_smp.e'],
                USE_OLD_FLOOR : True,
                CUSTOM_CMP : 'elastic_thermal_patch.cmp'}

test_jac_rz_smp = { INPUT : 'elastic_thermal_jacobian_rz_smp_test.i',
                    EXODIFF : ['out_jac_rz_smp.e'],
                    USE_OLD_FLOOR : True,
                    CUSTOM_CMP : 'elastic_thermal_patch.cmp'}
