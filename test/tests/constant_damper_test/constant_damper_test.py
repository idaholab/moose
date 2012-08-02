from options import *

testdamper = { INPUT : 'constant_damper_test.i',
               EXODIFF : ['out.e'],
               SCALE_REFINE : 5
               }

testverifydamping = { INPUT : 'constant_damper_test.i' ,
                      EXPECT_OUT : 'NL step\s+8',
                      PREREQ : ['testdamper'] }

