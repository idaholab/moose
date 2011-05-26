import tools

def test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'material_test.i',['out.e'], dofs, np, n_threads)

#def dg_test(dofs=0, np=0, n_threads=0):
#  tools.executeAppAndDiff(__file__,'material_test_dg.i',['out_dg.e'], dofs, np, n_threads)

def coupled_material_test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'coupled_material_test.i',['out_coupled.e'], dofs, np, n_threads)

def adv_mat_couple_test(dofs=0, np=0, n_threads=0):
  tools.executeAppAndDiff(__file__,'adv_mat_couple_test.i',['out_adv_coupled.e'], dofs, np, n_threads)

def mat_cyclic_dep_error_test():
  tools.executeAppExpectError(__file__,'mat_cyclic_coupling.i',"Cyclic dependency detected")

try:
  from options import *

  test = { INPUT : 'material_test.i',
           EXODIFF : ['out.e'] }

  dg_test = { INPUT : 'material_test_dg.i',
             EXODIFF : ['out_dg.e'],
             SKIP : True }

  coupled_material_test = { INPUT : 'coupled_material_test.i',
                          EXODIFF : ['out_coupled.e'] }

  adv_mat_couple_test = { INPUT : 'adv_mat_couple_test.i',
                       EXODIFF : ['out_adv_coupled.e'] }

  mat_cyclic_dep_error_test = { INPUT : 'mat_cyclic_coupling.i',
                            EXPECT_ERR : 'Cyclic dependency detected' }

  
except:
  pass
