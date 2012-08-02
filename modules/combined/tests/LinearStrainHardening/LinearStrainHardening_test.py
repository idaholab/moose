from options import *

testRestart1 = { INPUT : 'LinearStrainHardeningRestart1.i',
                 ABS_ZERO : 1e-9
               }

testRestart2 = { INPUT : 'LinearStrainHardeningRestart2.i',
                 EXODIFF : ['out.e'],
                 EXODIFF_OPTS : ['-TM'],
                 ABS_ZERO : 1e-9,
                 PREREQ : ['testRestart1']
               }

test = { INPUT : 'LinearStrainHardening_test.i',
         EXODIFF : ['out.e'],
         ABS_ZERO : 1e-9,
         SCALE_REFINE : 2,
         PREREQ : ['testRestart2']
         }
