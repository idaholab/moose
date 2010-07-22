import tools

def test(dofs=0, np=0):
  tools.executeAppAndDiffCSV(__file__,'element_h1_error_pp_test.i',['out.csv'])
 

