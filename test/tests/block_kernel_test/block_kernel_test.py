import tools

def test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'block_kernel_test.i',['out.e'], dofs, np, n_threads)

def testvars(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'block_vars.i',['out_vars.e'], dofs, np, n_threads)
  
try:
  from options import *

  test = { INPUT : 'block_kernel_test.i',
           EXODIFF : ['out.e'] }

  testvars = { INPUT : 'block_vars.i',
               EXODIFF : ['out_vars.e'] }

except:
  pass
