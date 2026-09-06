[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 50
    ny = 50
    xmax = 40
    ymax = 40
    elem_type = QUAD4
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
    type = KokkosDiffusion
    variable = u
  []

  [forcing]
    type = KokkosGaussContForcing
    variable = u
  []

  [dot]
    type = KokkosTimeDerivative
    variable = u
  []
[]

[BCs]
  [Periodic]
    [x]
      variable = u
      primary = 3
      secondary = 1
      translation = '40 0 0'
    []
    [y]
      variable = u
      primary = 0
      secondary = 2
      translation = '0 40 0'
    []
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 20
  solve_type = NEWTON
[]

[Outputs]
  execute_on = TIMESTEP_END
  exodus = true
[]
