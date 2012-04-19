from options import *

test = { INPUT : 'plenum_pressure.i',
         EXODIFF : ['out.e'],
         ABS_ZERO : 1e-10 }

test_rz = { INPUT : 'plenum_pressure_rz.i',
            EXODIFF : ['out_rz.e'],
            ABS_ZERO : 1e-10 }

test_refab = { INPUT : 'plenum_pressure_refab.i',
               EXODIFF : ['plenum_pressure_refab_out.e'],
               ABS_ZERO : 1e-10 }

