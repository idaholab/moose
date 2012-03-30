from options import *

test_1 = { INPUT : 'restart_diffusion_test_steady.i',
           EXODIFF : ['steady_out.e'] }

test_2 = { INPUT : 'restart_diffusion_test_transient.i',
           EXODIFF : ['out.e'],
           PREREQ : 'test_1' }

uniform_refine_test_1 = {
  INPUT : 'exodus_refined_restart_1_test.i',
  EXODIFF : ['exodus_refined_restart_1.e']
}

uniform_refine_test_2 = {
  INPUT : 'exodus_refined_restart_2_test.i',
  EXODIFF : ['exodus_refined_restart_2.e'],
  PREREQ : 'uniform_refine_test_1'
}

uniform_refine_refine_test_2 = {
  INPUT : 'exodus_refined_refined_restart_2_test.i',
  EXODIFF : ['exodus_refined_refined_restart_2.e'],
  PREREQ : 'uniform_refine_test_1'
}

