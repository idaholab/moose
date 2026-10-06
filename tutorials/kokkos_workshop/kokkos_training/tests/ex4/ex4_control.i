# Exercise 4 bonus: the controllable coefficient of KokkosConstantConvectiveBC ramped in time.
# There is no time derivative, so every time step is a steady solve with a new h = 2 t.
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 50
    ny = 50
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = KokkosNonlinearDiffusion
    variable = u
    k0 = 1
    beta = 2
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
  [top]
    type = KokkosConstantConvectiveBC
    variable = u
    boundary = top
    h = 0
    u_inf = 0.2
  []
[]

[Functions]
  [ramp]
    type = ParsedFunction
    expression = '2 * t'
  []
[]

[Controls]
  [ramp_h]
    type = RealFunctionControl
    parameter = 'BCs/top/h'
    function = ramp
    execute_on = 'initial timestep_begin'
  []
[]

[Postprocessors]
  [u_mid]
    type = PointValue
    variable = u
    point = '0.5 0.5 0'
  []
  [u_top]
    type = KokkosSideAverageValue
    variable = u
    boundary = top
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  nl_rel_tol = 1e-10
  dt = 1
  num_steps = 3
[]

[Outputs]
  csv = true
[]
