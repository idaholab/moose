import tools

def test(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'2_d_mms_sine_test.i',['2_d_out.e'], dofs, np)

def test(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'3_d_mms_sine_test.i',['3_d_out.e'], dofs, np)


def test(dofs=0, np=0):
  tools.executeAppAndDiffCSV(__file__,'2_d_mms_sine_postprocessor_test.i',['2_d_postprocessor_out.csv'])


def test(dofs=0, np=0):
  tools.executeAppAndDiffCSV(__file__,'3_d_mms_sine_postprocessor_test.i',['3_d_postprocessor_out.csv'])
