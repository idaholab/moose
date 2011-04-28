import tools

def testdirichlet(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'2d_diffusion_test.i',['out.e'], dofs, np, n_threads)

def testneumann(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'2d_diffusion_neumannbc_test.i',['neumannbc_out.e'], dofs, np, n_threads)


try: # temporary to test the new test harness
  from options import *

  test_dirichlet = { INPUT : '2d_diffusion_test.i',
                     EXODIFF : ['out.e'] }

  test_neumann   = { INPUT : '2d_diffusion_neumannbc_test.i',
                     EXODIFF : ['neumannbc_out.e'] }

  test_show_skip = { INPUT : '2d_diffusion_test.i',
                     EXODIFF : ['../constant_ic_test/gold/out.e'],
                     SKIP : True }

  test_show_csv_fail = { INPUT : '2d_diffusion_test.i',
                         CSVDIFF : ['not_here.csv'] }

  test_show_exo_fail = { INPUT : '2d_diffusion_test.i',
                         EXODIFF : ['not_here.e'] }

  test_show_errmsg = { INPUT : 'not_here.i',
                       EXODIFF : ['out.e'] }
except:
  pass
