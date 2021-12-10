[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 0.1e6
  initial_vel = 0
  initial_T = 300

  scaling_factor_1phase = '1. 1. 1.'

  closures = simple_closures
[]

[FluidProperties]
  [eos]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
    k = 0.5
    mu = 281.8e-6
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
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10

    A = 1.907720E-04
    f = 0.0

    fp = eos
  []

  [inlet]
    type = InletDensityVelocity1Phase
    input = 'pipe:in'

    rho = 996.556340388366266
    vel = 2
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 0.1e6
  []
[]

[Functions]
  [inlet_rho_fn]
    type = PiecewiseLinear
    x = '0   1 '
    y = '996 997'
  []

  [inlet_vel_fn]
    type = PiecewiseLinear
    x = '1 2'
    y = '1 2'
  []
[]

[ControlLogic]
  [inlet_rho_ctrl]
    type = TimeFunctionComponentControl
    component = inlet
    parameter = rho
    function = inlet_rho_fn
  []

  [inlet_vel_ctrl]
    type = TimeFunctionComponentControl
    component = inlet
    parameter = vel
    function = inlet_vel_fn
  []
[]

[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  dt = 0.1
  num_steps = 20
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 30

  l_tol = 1e-3
  l_max_its = 100
[]

[Postprocessors]
  [rho_inlet]
    type = RealComponentParameterValuePostprocessor
    component = inlet
    parameter = rho
  []
  [vel_inlet]
    type = RealComponentParameterValuePostprocessor
    component = inlet
    parameter = vel
  []
[]

[Outputs]
  csv = true
[]
