[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    xmax = 3
    x_max = 4 # Changed from 3 to 4
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = u
    boundary = left
    value = 300
  []
  [right]
    type = ADNeumannBC
    variable = u
    boundary = right
    value = 100
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  csv = true
[]
