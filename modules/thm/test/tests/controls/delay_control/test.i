[GlobalParams]
  initial_p = 100.e3
  initial_vel = 0
  initial_T = 300.
  closures = simple_closures
[]

[Functions]
  [p0_fn]
    type = PiecewiseLinear
    x = '0   0.2     0.4     0.6     0.8'
    y = '1e5 1.002e5 1.002e5 1.001e5 1.001e5'
  []
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [pipe1]
    type = FlowChannel1Phase
    fp = fp
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 5
    A   = 0.01
    D_h = 0.1
    f = 0
  []

  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe1:in'
    p0 = 100.e3
    T0 = 300.
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe1:out'
    p = 100.0e3
  []
[]

[ControlLogic]
  [p0_fn_ctrl]
    type = TimeFunctionComponentControl
    component = inlet
    parameter = p0
    function = p0_fn
  []

  [delay_ctrl]
    type = DelayControl
    input = p0_inlet
    tau = 0.3
    initial_value = 1e5
  []
[]

[Postprocessors]
  [p0_inlet_delayed]
    type = RealControlDataValuePostprocessor
    control_data_name = delay_ctrl:value
    execute_on = 'initial timestep_end'
  []

  [p0_inlet]
    type = FunctionValuePostprocessor
    function = p0_fn
    execute_on = 'initial timestep_begin'
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
  dt = 0.1
  start_time = 0.0
  end_time = 1.0
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 20

  l_tol = 1e-3
  l_max_its = 5

  automatic_scaling = true
[]

[Outputs]
  csv = true
[]
