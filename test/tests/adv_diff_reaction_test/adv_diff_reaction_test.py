import tools

def test(dofs=0, np=0):
  tools.executeAppAndDiff(__file__,'adv_diff_reaction_test.i',['out.e'])
