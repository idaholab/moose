from options import *

test = { INPUT : 'material_test.i',
         EXODIFF : ['out.e'],
         SCALE_REFINE : 3
         }

dg_test = { INPUT : 'material_test_dg.i',
            EXODIFF : ['out_dg.e'],
            SCALE_REFINE : 3
            }

coupled_material_test = { INPUT : 'coupled_material_test.i',
                          EXODIFF : ['out_coupled.e'],
                          SCALE_REFINE : 3
                          }

three_coupled_mat_test = { INPUT : 'three_coupled_mat_test.i',
                           EXODIFF : ['out_three.e'],
                           SCALE_REFINE : 3
                           }

adv_mat_couple_test = { INPUT : 'adv_mat_couple_test.i',
                        EXODIFF : ['out_adv_coupled.e'],
                        SCALE_REFINE : 4
                        }

adv_mat_couple_test2 = { INPUT : 'adv_mat_couple_test2.i',
                         EXODIFF : ['out_adv_coupled2.e'],
                         SCALE_REFINE : 4
                         }

mat_cyclic_dep_error_test = { INPUT : 'mat_cyclic_coupling.i',
                              EXPECT_ERR : 'Cyclic dependency detected' }


