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
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
  [../]
[]

# Materials properties into AuxVariables - these are elemental variables, not nodal variables.
[AuxVariables]
  [./C11_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C12_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C13_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C14_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C15_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C16_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C22_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C23_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C24_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C25_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C26_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C33_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C34_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C35_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C36_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C44_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C45_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C46_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C55_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C56_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C66_aux]
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
    variable = C11_aux
  [../]

  [./matl_C12]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 1
    variable = C12_aux
  [../]

  [./matl_C13]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 2
    variable = C13_aux
  [../]

  [./matl_C14]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 3
    variable = C14_aux
  [../]

  [./matl_C15]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 4
    variable = C15_aux
  [../]

  [./matl_C16]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 5
    variable = C16_aux
  [../]

  [./matl_C22]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 6
    variable = C22_aux
  [../]

  [./matl_C23]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 7
    variable = C23_aux
  [../]

  [./matl_C24]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 8
    variable = C24_aux
  [../]

  [./matl_C25]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 9
    variable = C25_aux
  [../]

  [./matl_C26]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 10
    variable = C26_aux
  [../]

  [./matl_C33]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 11
    variable = C33_aux
  [../]

  [./matl_C34]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 12
    variable = C34_aux
  [../]

  [./matl_C35]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 13
    variable = C35_aux
  [../]

  [./matl_C36]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 14
    variable = C36_aux
  [../]

  [./matl_C44]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 15
    variable = C44_aux
  [../]

  [./matl_C45]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 16
    variable = C45_aux
  [../]

  [./matl_C46]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 17
    variable = C46_aux
  [../]

  [./matl_C55]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 18
    variable = C55_aux
  [../]

  [./matl_C56]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 19
    variable = C56_aux
  [../]

  [./matl_C66]
    type = MaterialSymmElasticityTensorAux
    tensor_matpro = elasticity_tensor
    index = 20
    variable = C66_aux
  [../]
[]


[Materials]
  [./Anisotropic]
    type = LinearGeneralAnisotropicMaterial
    disp_x = disp_x
    disp_y = disp_y
    block = 0

    all_21 = true
    # reading   C_11  C_12  C_13  C_22  C_23  C_33  C_44  C_55  C_66
    # Tetragonal
    #C_matrix ='1.0e6 0.5e6 0.5e6 1.0e6 0.5e6 6.0e6 0.5e6 0.5e6 1.5e6'
    # Cubic
#    C_matrix ='1.0e6 0.5e6 0.5e6 1.0e6 0.5e6 1.0e6 0.5e6 0.5e6 0.5e6'
    # This is a test Cijkl entry to use with all_21 = true
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

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


[]

[Outputs]
  file_base = Linear_General_Anisotropic_Material_out
  exodus = true
[]
