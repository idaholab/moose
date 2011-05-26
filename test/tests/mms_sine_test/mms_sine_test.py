import tools

def test2dMesh(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'2_d_mms_sine_test.i',['2_d_out.e'], dofs, np, n_threads)

def test3dMesh(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'3_d_mms_sine_test.i',['3_d_out.e'], dofs, np, n_threads)


def test2dCSV(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiffCSV(__file__,'2_d_mms_sine_postprocessor_test.i',['2_d_postprocessor_out.csv'], dofs, np, n_threads)


def test3dCSV(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiffCSV(__file__,'3_d_mms_sine_postprocessor_test.i',['3_d_postprocessor_out.csv'], dofs, np, n_threads)

try:
  from options import *

  test2dMesh = { INPUT : '2_d_mms_sine_test.i',
                 EXODIFF : ['2_d_out.e'] }

  test3dMesh = { INPUT : '3_d_mms_sine_test.i',
                 EXODIFF : ['3_d_out.e'] }

  test2dCSV = { INPUT : '2_d_mms_sine_postprocessor_test.i',
                CSVDIFF : ['2_d_postprocessor_out.csv'] }

  test3dCSV = { INPUT : '3_d_mms_sine_postprocessor_test.i',
                CSVDIFF : ['3_d_postprocessor_out.csv'] }

except:
  pass
