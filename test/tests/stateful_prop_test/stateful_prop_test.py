import tools

def test():
  tools.executeAppAndDiff(__file__,'stateful_prop_test.i',['out.e'])
