# This input file is designed to test the RankTwoAux and RankFourAux
# auxkernels, which report values out of the Tensors used in materials
# properties.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  nz = 0
  xmin = 0
  xmax = 2
  ymin = 0
  ymax = 2
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./diffused]
    order = FIRST
    family = LAGRANGE
     [./InitialCondition]
      type = RandomIC
     [../]
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

# Materials properties into AuxVariables - these are elemental variables, not nodal variables.
[AuxVariables]
  [./C1111_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C1122_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C1133_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C1123_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C1113_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C1112_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C2222_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C2233_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C2223_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C2213_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C2212_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C3333_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C3323_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C3313_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C3312_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C2323_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C2313_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C2312_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C1313_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C1312_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C1212_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

#stresses
  [./s11_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./s12_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./s13_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./s22_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./s23_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./s33_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./TensorMechanics]
  [../]

  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]

[AuxKernels]
  [./matl_C1111]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 0
    index_l = 0
    variable = C1111_aux
  [../]

   [./matl_C1122]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 1
    index_l = 1
    variable = C1122_aux
  [../]

  [./matl_C1133]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 2
    index_l = 2
    variable = C1133_aux
  [../]

  [./matl_C1123]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 1
    index_l = 2
    variable = C1123_aux
  [../]

  [./matl_C1113]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 0
    index_l = 2
    variable = C1113_aux
  [../]

  [./matl_C1112]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 0
    index_l = 1
    variable = C1112_aux
  [../]

  [./matl_C2222]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 1
    index_j = 1
    index_k = 1
    index_l = 1
    variable = C2222_aux
  [../]

  [./matl_C2233]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 1
    index_j = 1
    index_k = 2
    index_l = 2
    variable = C2233_aux
  [../]

  [./matl_C2223]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 1
    index_j = 1
    index_k = 1
    index_l = 2
    variable = C2223_aux
  [../]

  [./matl_C2213]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 1
    index_j = 1
    index_k = 0
    index_l = 2
    variable = C2213_aux
  [../]

  [./matl_C2212]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 1
    index_j = 1
    index_k = 0
    index_l = 1
    variable = C2212_aux
  [../]

 [./matl_C3333]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 2
    index_j = 2
    index_k = 2
    index_l = 2
    variable = C3333_aux
  [../]

  [./matl_C3323]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 2
    index_j = 2
    index_k = 1
    index_l = 2
    variable = C3323_aux
  [../]

  [./matl_C3313]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 2
    index_j = 2
    index_k = 0
    index_l = 2
    variable = C3313_aux
  [../]

  [./matl_C3312]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 2
    index_j = 2
    index_k = 0
    index_l = 1
    variable = C3312_aux
  [../]

  [./matl_C2323]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 1
    index_j = 2
    index_k = 1
    index_l = 2
    variable = C2323_aux
  [../]

  [./matl_C2313]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 1
    index_j = 2
    index_k = 0
    index_l = 2
    variable = C2313_aux
  [../]

  [./matl_C2312]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 1
    index_j = 2
    index_k = 0
    index_l = 1
    variable = C2312_aux
  [../]

  [./matl_C1313]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 2
    index_k = 0
    index_l = 2
    variable = C1313_aux
  [../]

  [./matl_C1312]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 2
    index_k = 0
    index_l = 1
    variable = C1312_aux
  [../]

  [./matl_C1212]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 1
    index_k = 0
    index_l = 1
    variable = C1212_aux
  [../]

  [./matl_s11]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    variable = s11_aux
  [../]

 [./matl_s12]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 1
    variable = s12_aux
  [../]

  [./matl_s13]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 2
    variable = s13_aux
  [../]

  [./matl_s22]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
    variable = s22_aux
  [../]

  [./matl_s23]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 2
    variable = s23_aux
  [../]

  [./matl_s33]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    variable = s33_aux
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric21
    C_ijkl ='1111 1122 1133 1123 1113 1112 2222 2233 2223 2213 2212 3333 3323 3313 3312 2323 2313 2312 1313 1312 1212'
  [../]
  [./strain]
    type = ComputeSmallStrain
    block = 0
    displacements = 'disp_x disp_y'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = 0
  [../]
[]

[BCs]
  [./bottom]
    type = PresetBC
    variable = diffused
    boundary = '1'
    value = 1
  [../]

  [./top]
    type = PresetBC
    variable = diffused
    boundary = '2'
    value = 0
  [../]

  [./disp_x_BC]
    type = PresetBC
    variable = disp_x
    boundary = '0 2'
    value = 0.5
  [../]

  [./disp_x_BC2]
    type = PresetBC
    variable = disp_x
    boundary = '1 3'
    value = 0.01
  [../]

  [./disp_y_BC]
    type = PresetBC
    variable = disp_y
    boundary = '0 2'
    value = 0.8
  [../]

  [./disp_y_BC2]
    type = PresetBC
    variable = disp_y
    boundary = '1 3'
    value = 0.02
  [../]
[]

[Executioner]
  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_rel_tol = 1e-10
[]

[Outputs]
  file_base = Tensor_test
  exodus = true
[]
