[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Variables]
  [temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 2
  []
[]

[Kernels]
  [heat]
    type = KokkosHeatConduction
    variable = temp
  []

  [ie]
    type = KokkosHeatConductionTimeDerivative
    variable = temp
  []
[]

[BCs]
  [bottom]
    type = KokkosDirichletBC
    variable = temp
    boundary = 1
    value = 4
  []

  [top]
    type = KokkosDirichletBC
    variable = temp
    boundary = 2
    value = 1
  []
[]

[Materials]
  [mat]
    type = KokkosHeatConductionDerivativeMaterial
    temp = temp
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 1
  dt = .1
  nl_max_its = 10
  dtmin = .1
[]

[Outputs]
  exodus = true
[]
