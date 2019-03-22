# This input file is designed to test the LinearGeneralAnisotropicMaterial class.  This test is
# for regression testing.  This just takes the material properties and puts them into
# aux variables; the diffusion kernel is just to have a simple kernel to run the test.
[GlobalParams]
  displacements = 'disp_x disp_y'
[]

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
[]

[Modules/TensorMechanics/Master/All]
  strain = SMALL
  incremental = true
  add_variables = true
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
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 0
    index_l = 0
    variable = C11
  [../]
  [./matl_C12]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 0
   index_j = 0
   index_k = 1
   index_l = 1
   variable = C12
 [../]
 [./matl_C13]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 0
   index_j = 0
   index_k = 2
   index_l = 2
   variable = C13
 [../]
 [./matl_C14]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 0
   index_j = 0
   index_k = 1
   index_l = 2
   variable = C14
 [../]
 [./matl_C15]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 0
   index_j = 0
   index_k = 0
   index_l = 2
   variable = C15
 [../]
 [./matl_C16]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 0
   index_j = 0
   index_k = 0
   index_l = 1
   variable = C16
 [../]
 [./matl_C22]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 1
   index_j = 1
   index_k = 1
   index_l = 1
   variable = C22
 [../]
 [./matl_C23]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 1
   index_j = 1
   index_k = 2
   index_l = 2
   variable = C23
 [../]

 [./matl_C24]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 1
   index_j = 1
   index_k = 1
   index_l = 2
   variable = C24
 [../]
 [./matl_C25]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 1
   index_j = 1
   index_k = 0
   index_l = 2
   variable = C25
 [../]
 [./matl_C26]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 1
   index_j = 1
   index_k = 0
   index_l = 1
   variable = C26
 [../]
 [./matl_C33]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 2
   index_j = 2
   index_k = 2
   index_l = 2
   variable = C33
 [../]
 [./matl_C34]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 2
   index_j = 2
   index_k = 1
   index_l = 2
   variable = C34
 [../]
 [./matl_C35]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 2
   index_j = 2
   index_k = 0
   index_l = 2
   variable = C35
 [../]
 [./matl_C36]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 2
   index_j = 2
   index_k = 0
   index_l = 1
   variable = C36
 [../]

 [./matl_C44]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 1
   index_j = 2
   index_k = 1
   index_l = 2
   variable = C44
 [../]
 [./matl_C45]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 1
   index_j = 2
   index_k = 0
   index_l = 2
   variable = C45
 [../]
 [./matl_C46]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 1
   index_j = 2
   index_k = 0
   index_l = 1
   variable = C46
 [../]
 [./matl_C55]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 0
   index_j = 2
   index_k = 0
   index_l = 2
   variable = C55
 [../]
 [./matl_C56]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 0
   index_j = 2
   index_k = 0
   index_l = 1
   variable = C56
 [../]
 [./matl_C66]
   type = RankFourAux
   rank_four_tensor = elasticity_tensor
   index_i = 0
   index_j = 1
   index_k = 0
   index_l = 1
   variable = C66
 [../]
[]


[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric21
    C_ijkl ='1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0 13.0 14.0 15.0 16.0 17.0 18.0 19.0 20.0 21.0'
  [../]
  [./stress]
    type = ComputeStrainIncrementBasedStress
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
  exodus = true
[]
