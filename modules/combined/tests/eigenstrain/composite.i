[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]

[AuxVariables]
  [./c]
    [./InitialCondition]
      type = FunctionIC
      function = x
    [../]
  [../]
  [./s11]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./s22]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./ds11]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./ds22]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./s11]
    type = RankTwoAux
    execute_on = initial
    variable = s11
    rank_two_tensor = eigenstrain
    index_i = 0
    index_j = 0
  [../]
  [./s22]
    type = RankTwoAux
    execute_on = initial
    variable = s22
    rank_two_tensor = eigenstrain
    index_i = 1
    index_j = 1
  [../]
  [./ds11]
    type = RankTwoAux
    execute_on = initial
    variable = ds11
    rank_two_tensor = delastic_strain/dc
    index_i = 0
    index_j = 0
  [../]
  [./ds22]
    type = RankTwoAux
    execute_on = initial
    variable = ds22
    rank_two_tensor = delastic_strain/dc
    index_i = 1
    index_j = 1
  [../]
[]

[Materials]
  [./eigen1]
    type = GenericConstantRankTwoTensor
    tensor_values = '1 -1 0 0 0 0'
    tensor_name = eigen1
  [../]
  [./eigen2]
    type = GenericConstantRankTwoTensor
    tensor_values = '-1 1 0 0 0 0'
    tensor_name = eigen2
  [../]
  [./weight1]
    type = DerivativeParsedMaterial
    function = 0.02*c^2
    f_name = weight1
    args = c
  [../]
  [./weight2]
    type = DerivativeParsedMaterial
    function = 0.02*(1-c)^2
    f_name = weight2
    args = c
  [../]

  [./eigenstrain]
    type = CompositeEigenstrain
    tensors = 'eigen1  eigen2'
    weights = 'weight1 weight2'
    args = c
    block = 0
    eigenstrain_name = eigenstrain
  [../]
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
  execute_on = final
[]
