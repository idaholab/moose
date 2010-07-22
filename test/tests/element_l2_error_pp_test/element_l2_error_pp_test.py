import tools

def test(dofs=0, np=0):
  tools.executeAppAndDiffCSV(__file__,'element_l2_error_pp_test.i',['out.csv']) 

 
