[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [react]
    type = Reaction
    variable = u
  []
[]

[Materials]
  [tensor]
    type = AnisotropicRankTwoTensor
    tensor_name = 'A'
    value_1 = 1
    value_2 = 1
    value_3 = 1
    outputs = 'exodus'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]