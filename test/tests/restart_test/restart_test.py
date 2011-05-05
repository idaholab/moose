import tools

def test(dofs=0, np=0, n_threads=0):
    tools.executeAppAndDiff(__file__,'part1.i',['out_part1.e'], dofs, np, n_threads)
    tools.executeAppAndDiff(__file__,'part2.i',['out_part2.e'], dofs, np, n_threads)

def test_nodal_var(dofs=0, np=0, n_threads=0):
    tools.executeAppAndDiff(__file__,'part1.i',['out_part1.e'], dofs, np, n_threads)
    tools.executeAppAndDiff(__file__,'nodal_var_restart.i',['out_nodal_var_restart.e'], dofs, np, n_threads)
    