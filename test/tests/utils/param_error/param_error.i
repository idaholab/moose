[Mesh/gen]
  type = GeneratedMeshGenerator
  dim =1
[]

[Variables/u]
[]

[Kernels]
  [diffusion]
    type = CoeffParamDiffusion
    variable = u
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
