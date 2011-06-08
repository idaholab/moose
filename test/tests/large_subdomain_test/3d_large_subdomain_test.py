import tools

#Note: libMesh must be configured with 16-bit subdomain_id_type before enabling this test

def test(dofs=0, np=0, n_threads=0):
#  tools.executeAppAndDiff(__file__,'3d_large_subdomain_diffusion_test.i',['out.e'], dofs, np, n_threads)
  pass

try: # temporary to test the new test harness
  from options import *

  test = { INPUT : '3d_large_subdomain_diffusion_test.i',
           EXODIFF : ['out.e'],
           SKIP : True }
except:
  pass


