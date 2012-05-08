from options import *

test = { INPUT : 'penetration_locator_test.i',
         EXODIFF : ['out.e'] }

parallel_test = { INPUT : 'penetration_locator_test.i',
                  EXODIFF : ['out.e'],
                  PARALLEL : '4',
                  PREREQ : 'test' }

