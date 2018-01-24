# This input file is designed to test the LinearGeneralAnisotropicMaterial class.  This test is
# for regression testing.  This just takes the material properties and puts them into
# aux variables; the diffusion kernel is just to have a simple kernel to run the test.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 50
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [./diffused]
  [../]

  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    use_displaced_mesh = false
  [../]
[]

[AuxVariables]
  [./C11]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C12]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C13]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C14]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C15]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C16]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C22]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C23]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C24]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C25]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C26]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C33]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C34]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C35]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C36]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C44]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C45]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C46]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C55]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C56]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C66]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]

[AuxKernels]
  [./matl_C11]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 0
     variable = C11
   [../]
   [./matl_C12]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 1
     variable = C12
   [../]
   [./matl_C13]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 2
     variable = C13
   [../]
   [./matl_C14]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 3
     variable = C14
   [../]
   [./matl_C15]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 4
     variable = C15
   [../]
   [./matl_C16]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 5
     variable = C16
   [../]
   [./matl_C22]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 6
     variable = C22
   [../]
   [./matl_C23]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 7
     variable = C23
   [../]
   [./matl_C24]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 8
     variable = C24
   [../]
   [./matl_C25]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 9
     variable = C25
   [../]
   [./matl_C26]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 10
     variable = C26
   [../]
   [./matl_C33]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 11
     variable = C33
   [../]
   [./matl_C34]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 12
     variable = C34
   [../]
   [./matl_C35]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 13
     variable = C35
   [../]
   [./matl_C36]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 14
     variable = C36
   [../]
   [./matl_C44]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 15
     variable = C44
   [../]
   [./matl_C45]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 16
     variable = C45
   [../]
   [./matl_C46]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 17
     variable = C46
   [../]
   [./matl_C55]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 18
     variable = C55
   [../]
   [./matl_C56]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 19
     variable = C56
   [../]
   [./matl_C66]
     type = MaterialSymmElasticityTensorAux
     tensor_matpro = elasticity_tensor
     index = 20
     variable = C66
   [../]
[]

[Materials]
  [./Anisotropic]
    type = LinearGeneralAnisotropicMaterial
    disp_x = disp_x
    disp_y = disp_y
    block = 0

    all_21 = true
    C_matrix ='1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0 13.0 14.0 15.0 16.0 17.0 18.0 19.0 20.0 21.0'

    euler_angle_1 = 0.0
    euler_angle_2 = 0.0
    euler_angle_3 = 0.0
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = diffused
    boundary = '1'
    value = 1
  [../]
  [./top]
    type = DirichletBC
    variable = diffused
    boundary = '2'
    value = 0
  [../]
  [./disp_x_BC]
    type = DirichletBC
    variable = disp_x
    boundary = '0 1 2 3'
    value = 0.0
  [../]
  [./disp_y_BC]
    type = DirichletBC
    variable = disp_y
    boundary = '0 1 2 3'
    value = 0.0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  file_base = linear_anisotropic_material_out
  exodus = true
[]
