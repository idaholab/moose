from options import *

test = { INPUT : 'penetration_locator_test.i',
         EXODIFF : ['out.e'] }

parallel_test = { INPUT : 'penetration_locator_test.i',
                  EXODIFF : ['out.e'],
                  MAXPARALLEL : '3',
                  PREREQ : 'test' }

