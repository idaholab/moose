import tools

def test2dMesh():
  tools.executeAppAndDiff(__file__,'2_d_mms_sine_test.i',['2_d_out.e'])

def test3dMesh():
  tools.executeAppAndDiff(__file__,'3_d_mms_sine_test.i',['3_d_out.e'])


def test2dCSV():
  tools.executeAppAndDiffCSV(__file__,'2_d_mms_sine_postprocessor_test.i',['2_d_postprocessor_out.csv'])


def test3dCSV():
  tools.executeAppAndDiffCSV(__file__,'3_d_mms_sine_postprocessor_test.i',['3_d_postprocessor_out.csv'])
