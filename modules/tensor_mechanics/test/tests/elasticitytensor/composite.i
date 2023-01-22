# This input file is designed to test the RankTwoAux and RankFourAux
# auxkernels, which report values out of the Tensors used in materials
# properties.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmax = 1
[]

[AuxVariables]
  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = x
    [../]
  [../]

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

  [./C3313_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]


  [./dC1111_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./dC1122_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./dC1133_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./dC3313_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]


  [./d2C1111_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./d2C1122_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./d2C1133_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./d2C3313_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

#[Kernels]
#  [./diff]
#    type = Diffusion
#    variable = diffused
#  [../]
#[]

[AuxKernels]
  [./matl_C1111]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 0
    index_l = 0
    variable = C1111_aux
    execute_on = initial
  [../]

   [./matl_C1122]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 1
    index_l = 1
    variable = C1122_aux
    execute_on = initial
  [../]

  [./matl_C1133]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 2
    index_l = 2
    variable = C1133_aux
    execute_on = initial
  [../]

  [./matl_C3313]
    type = RankFourAux
    rank_four_tensor = elasticity_tensor
    index_i = 2
    index_j = 2
    index_k = 0
    index_l = 2
    variable = C3313_aux
    execute_on = initial
  [../]


  [./matl_dC1111]
    type = RankFourAux
    rank_four_tensor = delasticity_tensor/dc
    index_i = 0
    index_j = 0
    index_k = 0
    index_l = 0
    variable = dC1111_aux
    execute_on = initial
  [../]

   [./matl_dC1122]
    type = RankFourAux
    rank_four_tensor = delasticity_tensor/dc
    index_i = 0
    index_j = 0
    index_k = 1
    index_l = 1
    variable = dC1122_aux
    execute_on = initial
  [../]

  [./matl_dC1133]
    type = RankFourAux
    rank_four_tensor = delasticity_tensor/dc
    index_i = 0
    index_j = 0
    index_k = 2
    index_l = 2
    variable = dC1133_aux
    execute_on = initial
  [../]

  [./matl_dC3313]
    type = RankFourAux
    rank_four_tensor = delasticity_tensor/dc
    index_i = 2
    index_j = 2
    index_k = 0
    index_l = 2
    variable = dC3313_aux
    execute_on = initial
  [../]


  [./matl_d2C1111]
    type = RankFourAux
    rank_four_tensor = d^2elasticity_tensor/dc^2
    index_i = 0
    index_j = 0
    index_k = 0
    index_l = 0
    variable = d2C1111_aux
    execute_on = initial
  [../]

   [./matl_d2C1122]
    type = RankFourAux
    rank_four_tensor = d^2elasticity_tensor/dc^2
    index_i = 0
    index_j = 0
    index_k = 1
    index_l = 1
    variable = d2C1122_aux
    execute_on = initial
  [../]

  [./matl_d2C1133]
    type = RankFourAux
    rank_four_tensor = d^2elasticity_tensor/dc^2
    index_i = 0
    index_j = 0
    index_k = 2
    index_l = 2
    variable = d2C1133_aux
    execute_on = initial
  [../]

  [./matl_d2C3313]
    type = RankFourAux
    rank_four_tensor = d^2elasticity_tensor/dc^2
    index_i = 2
    index_j = 2
    index_k = 0
    index_l = 2
    variable = d2C3313_aux
    execute_on = initial
  [../]
[]

[Materials]
  [./Ca]
    type = ComputeElasticityTensor
    base_name = Ca
    block = 0
    fill_method = symmetric21
    C_ijkl ='1111 .1122 1133 1123 1113 1112 2222 2233 2223 2213 2212 3333 3323 3313 3312 2323 2313 2312 1313 1312 1212'
  [../]
  [./Cb]
    type = ComputeElasticityTensor
    base_name = Cb
    block = 0
    fill_method = symmetric21
    C_ijkl ='.1111 1122 .1133 .1123 .1113 .1112 .2222 .2233 .2223 .2213 .2212 .3333 .3323 .3313 .3312 .2323 .2313 .2312 .1313 .1312 .1212'
  [../]
  [./Fa]
    type = DerivativeParsedMaterial
    block = 0
    property_name = Fa
    expression = c^2
    coupled_variables = c
  [../]
  [./Fb]
    type = DerivativeParsedMaterial
    block = 0
    property_name = Fb
    expression = (1-c)^3
    coupled_variables = c
  [../]
  [./C]
    type = CompositeElasticityTensor
    block = 0
    args = c
    tensors = 'Ca Cb'
    weights = 'Fa Fb'
  [../]
[]
[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
