from options import *

test = { INPUT : 'gravity_test.i',
         EXODIFF : ['out.e'],
         USE_OLD_FLOOR : True,
         ABS_ZERO : 1e-9 }

test_hex20 = { INPUT : 'gravity_hex20_test.i',
               EXODIFF : ['out_hex20.e'],
               USE_OLD_FLOOR : True,
               ABS_ZERO : 1e-9 }

test_rz = { INPUT : 'gravity_rz_test.i',
            EXODIFF : ['out_rz.e'],
            USE_OLD_FLOOR : True,
            ABS_ZERO : 1e-9 }

test_rz_quad8 = { INPUT : 'gravity_rz_quad8_test.i',
                  EXODIFF : ['out_rz_quad8.e'],
                  USE_OLD_FLOOR : True,
                  ABS_ZERO : 1e-9 }

