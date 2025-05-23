AD = ''

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = ${AD}Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = ${AD}DirichletBC
    variable = u
    boundary = 3
    value = 0
  []
  [right]
    type = ${AD}NeumannBC
    variable = u
    boundary = 1
    value = 1
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
