import tools

def test(dofs=0, np=0):
  tools.executeAppAndDiffCSV(__file__,'element_average_value_test.i',['out.csv'], dofs, np)
