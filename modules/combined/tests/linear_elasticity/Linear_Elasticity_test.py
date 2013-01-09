from options import *

linear_general_anisotropic_elasticity_test = { INPUT : 'Linear_Material_test.i',
         EXODIFF : ['Linear_General_Anisotropic_Material_out.e']}

linear_elastic_material_test = { INPUT : 'LinearElasticMaterial_test.i',
	 EXODIFF : ['LinearElasticMaterial.e']}

tensor_test = { INPUT : 'Tensor_test.i',
         EXODIFF : ['Tensor_test.e']}

th_exp_test = { INPUT : 'thermal_expansion_test.i',
         EXODIFF : ['thermal_expansion.e']}

appl_str_test = { INPUT : 'applied_strain_test.i',
         EXODIFF : ['applied_strain.e']}
