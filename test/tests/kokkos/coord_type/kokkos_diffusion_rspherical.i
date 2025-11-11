[Mesh]
  [sphere]
    type = GeneratedMeshGenerator
    nx = 10
    dim = 1
  []
  coord_type = RSPHERICAL
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = 0
    value = 1
  []
  [right]
    type = KokkosDirichletBC
    variable = u
    boundary = 1
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
