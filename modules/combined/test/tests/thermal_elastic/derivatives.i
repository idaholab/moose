[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  [./a]
    [./InitialCondition]
      type = RandomIC
      min = -1
      max = 1
    [../]
  [../]
  [./b]
    [./InitialCondition]
      type = RandomIC
      min = -1
      max = 1
    [../]
  [../]
[]

[Debug]
  [./MaterialDerivativeTest]
    [./elastic]
      prop_name = elasticity_tensor
      prop_type = RankFourTensor
      derivative_order = 1
      args = 'a b'
    [../]
  [../]
[]

[Problem]
  kernel_coverage_check = false
[]

[Materials]
  [./youngs_modulus]
    type = DerivativeParsedMaterial
    property_name = youngs_modulus
    expression = '23.1 * a^4 + 10.7 * b^2'
    coupled_variables = 'a b'
  [../]
  [./poissons_ratio]
    type = DerivativeParsedMaterial
    property_name = poissons_ratio
    expression = '0.2 * a^2 + 0.29 * b^3'
    coupled_variables = 'a b'
  [../]

  [./elasticity_tensor]
    type = ComputeVariableIsotropicElasticityTensor
    args = 'a b'
    youngs_modulus = youngs_modulus
    poissons_ratio = poissons_ratio
  [../]
[]

[Executioner]
  type = Steady
[]
