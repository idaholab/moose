[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./TensorMechanics]
  [../]
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
    variable = s11
    rank_two_tensor = eigenstrain
    index_i = 0
    index_j = 0
  [../]
  [./s22]
    type = RankTwoAux
    variable = s22
    rank_two_tensor = eigenstrain
    index_i = 1
    index_j = 1
  [../]
  [./ds11]
    type = RankTwoAux
    variable = ds11
    rank_two_tensor = delastic_strain/dc
    index_i = 0
    index_j = 0
  [../]
  [./ds22]
    type = RankTwoAux
    variable = ds22
    rank_two_tensor = delastic_strain/dc
    index_i = 1
    index_j = 1
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '1 1'
    fill_method = symmetric_isotropic
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y'
    eigenstrain_names = 'eigenstrain'
  [../]
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
    expression = 0.02*c^2
    property_name = weight1
    coupled_variables = c
  [../]
  [./weight2]
    type = DerivativeParsedMaterial
    expression = 0.02*(1-c)^2
    property_name = weight2
    coupled_variables = c
  [../]

  [./eigenstrain]
    type = CompositeEigenstrain
    tensors = 'eigen1  eigen2'
    weights = 'weight1 weight2'
    args = c
    eigenstrain_name = eigenstrain
  [../]
[]

[BCs]
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
  execute_on = final
[]
