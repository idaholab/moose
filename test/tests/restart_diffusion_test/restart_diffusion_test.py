import tools

def test(dofs=0, np=0, n_threads=0):
    tools.executeAppAndDiff(__file__,'restart_diffusion_test_steady.i',['steady_out.e'], dofs, np, n_threads)
    tools.executeAppAndDiff(__file__,'restart_diffusion_test_transient.i',['out.e'], dofs, np, n_threads)

try:
    from options import *

    test_1 = { INPUT : 'restart_diffusion_test_steady.i',
               EXODIFF : ['steady_out.e'] }

    test_2 = { INPUT : 'restart_diffusion_test_transient.i',
               EXODIFF : ['out.e'],
               PREREQ : 'test_1' }

except:
    pass
