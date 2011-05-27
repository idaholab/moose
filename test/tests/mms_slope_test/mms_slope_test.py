import tools

def test():
  tools.executeAppAndDiff(__file__,'mms_slope_test.i',['out_0001.e','out_0006.e'])

  #TODO: execute the test and compare the post processor csv output
  #      to make sure the slope is -2
  pass

try:
  from options import *

  test = { INPUT : 'mms_slope_test.i',
           EXODIFF : ['out_0001.e','out_0006.e'] }

except:
  pass
