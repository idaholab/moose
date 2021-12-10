[GlobalParams]
  initial_T = 393.15
  initial_vel = 0.0372
  f = 0
  fp = fp

  scaling_factor_1phase = '1e-2 1e-2 1e-5'

  closures = simple_closures
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

[Functions]
  [pump_head_fn]
    type = PiecewiseLinear
    x = '0  0.5'
    y = '0  1  '
  []
[]

[Components]
  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe1:in'
    m_dot = 20
    T = 393.15
  []

  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10
    A = 0.567

    initial_p = 1.318964e+07
  []

  [pump]
    type = Pump1Phase
    connections = 'pipe1:out pipe2:in'
    position = '1.02 0 0'
    head = 0
    volume = 0.567
    A_ref = 0.567

    initial_p = 1.318964e+07
    initial_vel_x = 0.0372
    initial_vel_y = 0
    initial_vel_z = 0
    scaling_factor_rhoV  = 1
    scaling_factor_rhouV = 1
    scaling_factor_rhoEV = 1e-5
  []

  [pipe2]
    type = FlowChannel1Phase
    position = '1.04 0 0'
    orientation = '1 0 0'
    length = 0.96
    n_elems = 10
    A = 0.567

    initial_p = 1.4072e+07
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe2:out'
    p = 1.4072e+07
  []
[]

[ControlLogic]
  [pump_head_ctrl]
    type = TimeFunctionComponentControl
    component = pump
    parameter = head
    function = pump_head_fn
  []
[]

[Postprocessors]
  [pump_head]
    type = RealComponentParameterValuePostprocessor
    component = pump
    parameter = head
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

  start_time = 0
  dt = 0.1
  num_steps = 10
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  nl_max_its = 15
  l_tol = 1e-4
  l_max_its = 10

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Outputs]
  [out]
    type = CSV
    show = 'pump_head'
  []
  print_linear_residuals = false
[]
