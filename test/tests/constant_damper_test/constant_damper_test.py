import tools

def testdamper(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'constant_damper_test.i',['out.e'], dofs, np, n_threads)

# Make sure the damping causes 8 NL steps
def testverifydamping(dofs=0, np=0, n_threads=0):
  tools.executeAppExpectError(__file__,'constant_damper_test.i','NL step\s+8')

try:
  from options import *

  testdamper = { INPUT : 'constant_damper_test.i',
                 EXODIFF : ['out.e'] }

  testverifydamping = { INPUT : 'constant_damper_test.i' ,
                        EXPECT_ERR : 'NL step\s+8',
                        PREREQ : 'testdamper' }

except:
  pass
