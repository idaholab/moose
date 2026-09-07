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
  [time]
    type = KokkosTimeDerivative
    variable = u
  []
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
  type = Transient

  solve_type = NEWTON

  num_steps = 2
  dt = 0.1
[]

[Outputs]
  exodus = false
[]
