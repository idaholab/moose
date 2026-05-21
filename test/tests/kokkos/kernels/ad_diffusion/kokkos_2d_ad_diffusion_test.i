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
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = KokkosADDiffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = KokkosADDirichletBC
    variable = u
    boundary = 3
    value = 0
  []
  [right]
    type = KokkosADDirichletBC
    variable = u
    boundary = 1
    value = 1
  []
[]

[Executioner]
  type = Steady

  solve_type = NEWTON
  residual_and_jacobian_together = true
[]

[Outputs]
  exodus = true
[]
