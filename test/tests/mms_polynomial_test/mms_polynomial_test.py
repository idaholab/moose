import tools

def test():
  tools.executeAppAndDiff(__file__,'mms_polynomial_test.i',['out.e'])
