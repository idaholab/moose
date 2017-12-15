[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./slm]
    type = ScalarLagrangeMultiplier
    variable = u
    # 'b' does not exist -> error
    lambda = b
  [../]
[]

[Executioner]
  type = Steady
[]
