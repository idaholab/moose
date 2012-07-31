from options import *

test = { INPUT : 'element_aux_var_test.i',
         EXODIFF : ['out.e'] }

sort_test = { INPUT : 'elemental_sort_test.i',
              EXODIFF : ['elemental_sort_test_out.e'],
              SKIP : 'Waiting for Elem DOF update' }
