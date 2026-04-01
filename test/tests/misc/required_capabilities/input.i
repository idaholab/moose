[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
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
[]

[Problem]
  solve = False
[]

[Executioner]
  type = Steady
[]
