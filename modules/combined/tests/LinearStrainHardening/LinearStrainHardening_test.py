from options import *

test = { INPUT : 'LinearStrainHardening_test.i',
         EXODIFF : ['out.e'],
         ABS_ZERO : 1e-9,
         SCALE_REFINE : 2
         }

testRestart1 = { INPUT : 'LinearStrainHardeningRestart1.i',
                 EXODIFF : ['LinearStrainHardeningRestart1_out.e'],
                 ABS_ZERO : 1e-9,
                 PREREQ : 'test',
                 SKIP : 'Need exodiff option'
               }

testRestart2 = { INPUT : 'LinearStrainHardeningRestart2.i',
                 EXODIFF : ['out.e'],
#                 EXODIFF_OPTIONS : ['-TM'],
                 ABS_ZERO : 1e-9,
                 PREREQ : 'testRestart1',
                 SKIP : 'Need exodiff option'
               }

