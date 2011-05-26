import tools

def testie(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'ie.i',['out_ie.e'], dofs, np, n_threads)
  
def testbdf2(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'bdf2.i',['out_bdf2.e'], dofs, np, n_threads)

def testcranic(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'cranic.i',['out_cranic.e'], dofs, np, n_threads)


def testdt2(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'dt2.i',['out_dt2.e'], dofs, np, n_threads)

# versions with adaptivity

def test_ie_adapt(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'ie_adapt.i',['out_ie_adapt_0004.e'], dofs, np, n_threads)
  
def test_bdf2_adapt(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'bdf2_adapt.i',['out_bdf2_adapt_0004.e'], dofs, np, n_threads)

def test_cranic_adapt(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'cranic_adapt.i',['out_cranic_adapt_0004.e'], dofs, np, n_threads)

def testdt2(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'dt2_adapt.i',['out_dt2_adapt_0037.e'], dofs, np, n_threads)

# 
#def test_solution_time_adaptive(dofs=0, np=0, n_threads=0):
#  tools.executeAppAndDiff(__file__,'time-adaptive.i',['out_time_adaptive.e'], dofs, np, n_threads)

try:
  from options import *

  testie = { INPUT : 'ie.i',
             EXODIFF : ['out_ie.e'] }
  
  testbdf2 = { INPUT : 'bdf2.i',
               EXODIFF : ['out_bdf2.e'] }

  testcranic = { INPUT : 'cranic.i',
                 EXODIFF : ['out_cranic.e'] }

  testdt2 = { INPUT : 'dt2.i',
              EXODIFF : ['out_dt2.e'] }

# versions with adaptivity

  test_ie_adapt = { INPUT : 'ie_adapt.i',
                    EXODIFF : ['out_ie_adapt_0004.e'] }
  
  test_bdf2_adapt = { INPUT : 'bdf2_adapt.i',
                      EXODIFF : ['out_bdf2_adapt_0004.e'] }

  test_cranic_adapt = { INPUT : 'cranic_adapt.i',
                        EXODIFF : ['out_cranic_adapt_0004.e'] }

  testdt2 = { INPUT : 'dt2_adapt.i',
              EXODIFF : ['out_dt2_adapt_0037.e'] }

  test_solution_time_adaptive = { INPUT : 'time-adaptive.i',
                                  EXODIFF : ['out_time_adaptive.e'],
                                  SKIP : True }
except:
  pass
