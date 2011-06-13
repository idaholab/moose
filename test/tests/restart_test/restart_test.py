from options import *

test_1 = { INPUT : 'part1.i',
           EXODIFF : ['out_part1.e'] }

test_2 = { INPUT : 'part2.i',
           EXODIFF : ['out_part2.e'],
           PREREQ : 'test_1' }


test_nodal_var_1 = { INPUT : 'nodal_part1.i',
                     EXODIFF : ['out_nodal_part1.e'] }

test_nodal_var_2 = { INPUT : 'nodal_var_restart.i',
                     EXODIFF : ['out_nodal_var_restart.e'],
                     PREREQ : 'test_nodal_var_1' }

