import tools

def test_transient(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'transient.i',['out_transient.e'], dofs, np, n_threads)
  
def test_sln_time_adapt(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'sln-time-adapt.i',['out_sta.e'], dofs, np, n_threads)
