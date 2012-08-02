from options import *

test = { INPUT : 'extra_nodeset_test.i',
         EXODIFF : ['out.e'] }

test_coord =  { INPUT : 'extra_nodeset_coord_test.i',
                EXODIFF : ['out.e'],
                PREREQ : ['test'] }
