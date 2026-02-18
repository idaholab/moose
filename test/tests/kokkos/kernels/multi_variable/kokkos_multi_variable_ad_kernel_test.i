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
  [v]
    order = FIRST
    family = LAGRANGE
  []
  [w]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [time]
    type = KokkosADTimeDerivative
    variables = 'u v w'
  []
  [diff]
    type = KokkosADDiffusion
    variables = 'u v w'
  []
[]

[BCs]
  [left]
    type = KokkosADDirichletBC
    variables = 'u v w'
    boundary = 3
    value = 0
  []
  [right]
    type = KokkosADNeumannBC
    variables = 'v w'
    boundary = 1
    value = 1
  []
  [top]
    type = KokkosADNeumannBC
    variable = u
    boundary = 2
    value = 2
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 2

  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
