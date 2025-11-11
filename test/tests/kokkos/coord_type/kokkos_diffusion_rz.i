[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 10
    ny = 10
    dim = 2
  []
  coord_type = RZ
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
    boundary = 3
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
