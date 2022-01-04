# This test problem is the Lax shock tube test problem,
# which is a Riemann problem with the following parameters:
#   * domain = (0,1)
#   * gravity = 0
#   * EoS: Ideal gas EoS with gamma = 1.4, R = 0.71428571428571428571
#   * interface: x = 0.5
#   * typical end time: 0.15
# Left initial values:
#   * rho = 0.445
#   * vel = 0.692
#   * p = 3.52874226
# Right initial values:
#   * rho = 0.5
#   * vel = 0
#   * p = 0.571

[GlobalParams]
  gravity_vector = '0 0 0'

  rdg_slope_reconstruction = minmod

  closures = simple_closures
[]

[Functions]
  [p_ic_fn]
    type = PiecewiseConstant
    axis = x
    direction = right
    x = '0.5        1.0'
    y = '3.52874226 0.571'
  []

  [T_ic_fn]
    type = PiecewiseConstant
    axis = x
    direction = right
    x = '0.5              1.0'
    y = '11.1016610426966 1.5988'
  []

  [vel_ic_fn]
    type = PiecewiseConstant
    axis = x
    direction = right
    x = '0.5   1.0'
    y = '0.692 0.0'
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
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 100
    A = 1.0

    # IC
    initial_T = T_ic_fn
    initial_p = p_ic_fn
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
    # add order via 'cli_args' in 'tests'
  []
  solve_type = LINEAR

  l_tol = 1e-4

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 60

  # run to t = 0.15
  start_time = 0.0
  dt = 1e-3
  num_steps = 150
  abort_on_solve_fail = true
[]

[Outputs]
  file_base = 'lax_shock_tube'
  velocity_as_vector = false
  execute_on = 'initial timestep_end'
  [out]
    type = Exodus
    show = 'rho p vel'
  []
[]
