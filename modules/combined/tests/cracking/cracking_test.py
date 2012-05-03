from options import *

test = { INPUT : 'cracking.i',
         EXODIFF : ['cracking_out.e'],
         ABS_ZERO : 1e-4,
         SCALE_REFINE : 4
         }

test_rz = { INPUT : 'cracking_rz.i',
            EXODIFF : ['cracking_rz_out.e'],
            ABS_ZERO : 1e-4,
            SCALE_REFINE : 4
          }

test_exponential = { INPUT : 'cracking_exponential.i',
                     EXODIFF : ['cracking_exponential_out.e'],
                     ABS_ZERO : 1e-4,
                     SCALE_REFINE : 4
                    }

