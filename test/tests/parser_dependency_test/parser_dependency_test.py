import tools

def testmixed(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'parse_depend_mixed_test.i',['2d_diffusion_out.e'], dofs, np, n_threads)

def testreverse(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'parse_depend_reverse_test.i',['2d_diffusion_reverse_out.e'], dofs, np, n_threads)

def testpbp(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'parse_depend_pbp_test.i',['pbp_out.e'], dofs, np, n_threads)

try:
  from options import *

  testmixed = { INPUT : 'parse_depend_mixed_test.i',
                EXODIFF : ['2d_diffusion_out.e'] }

  testreverse = { INPUT : 'parse_depend_reverse_test.i',
                  EXODIFF : ['2d_diffusion_reverse_out.e'] }

  testpbp = { INPUT : 'parse_depend_pbp_test.i',
              EXODIFF : ['pbp_out.e'] }

except:
  pass
