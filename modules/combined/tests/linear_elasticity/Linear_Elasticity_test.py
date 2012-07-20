from options import *

linear_general_anisotropic_elasticity_test = { INPUT : 'Linear_Material_test.i',
         EXODIFF : ['Linear_General_Anisotropic_Material_out.e']}

linear_elastic_material_test = { INPUT : 'LinearElasticMaterial_test.i',
	 EXODIFF : ['LinearElasticMaterial.e']}

tensor_test = { INPUT : 'Tensor_test.i',
         EXODIFF : ['Tensor_test.e']}
