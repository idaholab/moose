[Mesh]
  file = trapezoid.e
  uniform_refine = 1
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [periodic_dist]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []

  [forcing]
    type = GaussContForcing
    variable = u
    x_center = 2
    y_center = -1
    x_spread = 0.25
    y_spread = 0.5
  []

  [dot]
    type = TimeDerivative
    variable = u
  []
[]

[AuxKernels]
  [periodic_dist]
    type = PeriodicDistanceAux
    variable = periodic_dist
    point = '0.2 1.7 0.0'
  []
[]

[BCs]
  [right]
    type = DirichletBC
    variable = u
    value = 1
    boundary = 2
  []
  [left]
    type = DirichletBC
    variable = u
    value = 2
    boundary = 2
  []
[]

[Executioner]
  type = Transient
  dt = 0.5
  num_steps = 6
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
