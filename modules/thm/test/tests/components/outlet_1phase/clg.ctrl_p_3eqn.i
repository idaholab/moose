[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 1e5
  initial_T = 300
  initial_vel = 0.0

  scaling_factor_1phase = '1 1 1e-5'

  closures = simple_closures
[]

[FluidProperties]
  [fp]
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
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 50

    A   = 1.0000000000e-04
    D_h = 1.1283791671e-02

    f = 0.0

    fp = fp
  []

  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe:in'
    p0 = 1e5
    T0 = 300
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 1e5
  []
[]

[Functions]
  [outlet_p_fn]
    type = PiecewiseLinear
    x = '0   1'
    y = '1e5 1.001e5'
  []
[]

[ControlLogic]
  [set_outlet_value]
    type = TimeFunctionComponentControl
    component = outlet
    parameter = p
    function = outlet_p_fn
  []
[]

[Postprocessors]
  [outlet_p]
    type = RealComponentParameterValuePostprocessor
    component = outlet
    parameter = p
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0.0
  dt = 0.25
  num_steps = 5
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  line_search = 'basic'

  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-6
  nl_max_its = 30

  l_tol = 1e-3
  l_max_its = 100

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Outputs]
  csv = true
[]
