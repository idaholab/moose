from options import *

test = { INPUT : 'finite_strain_elasticity_test.i',
         EXODIFF : ['out.e'],
         SCALE_REFINE : 1,
	 SKIP : 'FPE in FSMaterial.C:115'
         }


rotation_test = { INPUT : 'elastic_rotation_test.i',
         EXODIFF : ['elastic_rotation.e'],
         SCALE_REFINE : 1,
	 SKIP : 'FPE in FSMaterial.C:115'
         }  

