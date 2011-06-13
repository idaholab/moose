from options import *

testdamper = { INPUT : 'constant_damper_test.i',
               EXODIFF : ['out.e'] }

testverifydamping = { INPUT : 'constant_damper_test.i' ,
                      EXPECT_ERR : 'NL step\s+8',
                      PREREQ : 'testdamper' }

