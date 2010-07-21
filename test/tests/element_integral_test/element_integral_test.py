import tools

def test(dofs=0, np=0):
  tools.executeAppAndDiffCSV(__file__,'element_integral_test.i',['out.csv'])
