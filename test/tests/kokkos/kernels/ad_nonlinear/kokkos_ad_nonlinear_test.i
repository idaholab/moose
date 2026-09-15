[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = KokkosADNonlinearDiffusion
    variable = u
  []
  [reaction]
    type = KokkosADNonlinearReaction
    variable = u
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = KokkosDirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady

  solve_type = NEWTON
[]

[Outputs]
  exodus = false
[]
