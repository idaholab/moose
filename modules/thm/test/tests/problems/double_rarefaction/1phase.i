# Riemann problem that has a double-rarefaction solution

[GlobalParams]
  gravity_vector = '0 0 0'

  rdg_slope_reconstruction = minmod

  closures = simple_closures
[]

[Functions]
  [vel_ic_fn]
    type = PiecewiseConstant
    axis = x
    direction = right
    x = ' 0.0 0.1'
    y = '-1.0 1.0'
  []
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 11.64024372
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [pipe]
    type = FlowChannel1Phase

    fp = fp

    # geometry
    position = '-1 0 0'
    orientation = '1 0 0'
    length = 2.0
    n_elems = 100
    A = 1.0

    # IC
    initial_T = 0.04
    initial_p = 0.2
    initial_vel = vel_ic_fn

    f = 0
  []

  [left_boundary]
    type = FreeBoundary1Phase
    input = 'pipe:in'
  []

  [right_boundary]
    type = FreeBoundary1Phase
    input = 'pipe:out'
  []
[]

[Executioner]
  type = Transient
  [TimeIntegrator]
    type = ExplicitSSPRungeKutta
    order = 2
  []
  solve_type = LINEAR

  l_tol = 1e-4

  nl_rel_tol = 1e-20
  nl_abs_tol = 1e-8
  nl_max_its = 60

  # run to t = 0.6
  start_time = 0.0
  dt = 1e-3
  num_steps = 600
  abort_on_solve_fail = true
[]

[Outputs]
  file_base = '1phase'
  velocity_as_vector = false
  execute_on = 'initial timestep_end'
  [out]
    type = Exodus
    show = 'p T vel'
  []
[]
