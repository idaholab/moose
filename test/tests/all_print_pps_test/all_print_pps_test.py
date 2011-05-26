import tools

def test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiffCSV(__file__,'all_print_pps_test.i',['out.csv'],dofs,np,n_threads)

try:
  from options import *

  test = { INPUT : 'all_print_pps_test.i',
           CSVDIFF : ['out.csv'] }

except:
  pass
