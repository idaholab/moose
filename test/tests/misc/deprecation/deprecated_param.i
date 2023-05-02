[Mesh/gen]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Variables/u]
[]

[Kernels]
  [diffusion]
    type = DeprecatedParamDiffusion
    variable = u
    D = 4
  []

  [diffusion2]
    type = DeprecatedParamDiffusion
    variable = u
    D = 5
    E = 2
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
