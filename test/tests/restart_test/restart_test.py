import tools

def test(dofs=0, np=0, n_threads=0):
    tools.executeAppAndDiff(__file__,'part1.i',['out_part1.e'], dofs, np, n_threads)
    tools.executeAppAndDiff(__file__,'part2.i',['out_part2.e'], dofs, np, n_threads)

def test_nodal_var(dofs=0, np=0, n_threads=0):
    tools.executeAppAndDiff(__file__,'nodal_part1.i',['out_nodal_part1.e'], dofs, np, n_threads)
    tools.executeAppAndDiff(__file__,'nodal_var_restart.i',['out_nodal_var_restart.e'], dofs, np, n_threads)
    

try:
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
    
except:
    pass
