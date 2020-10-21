[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 0.1e6
  initial_vel = 0
  initial_T = 300

  scaling_factor_1phase = '1. 1. 1.'

  closures = simple

  spatial_discretization = cg
[]

[FluidProperties]
  [./eos]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
  [../]
[]

[Components]
  [./pipe]
    type = FlowChannel1Phase
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 100

    A = 1.907720E-04
    f = 0.0

    fp = eos
  [../]

  [./inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe:in'
    # will be set by control logic
    m_dot = 0
    T = 0
  [../]

  [./outlet]
    type = Outlet1Phase
    input = 'pipe:out'

    p = 0.1e6
  [../]
[]

[Functions]
  [./inlet_m_dot_fn]
    type = PiecewiseLinear
    x = '0 1 2'
    y = '0.38 0.38 0.4'
  [../]

  [./inlet_T_fn]
    type = PiecewiseLinear
    x = '0 1 2'
    y = '300 300 305'
  [../]
[]

[ControlLogic]
  [./inlet_m_dot_ctrl]
    type = TimeFunctionControl
    component = inlet
    parameter = m_dot
    function = inlet_m_dot_fn
  [../]

  [./inlet_T_ctrl]
    type = TimeFunctionControl
    component = inlet
    parameter = T
    function = inlet_T_fn
  [../]
[]

[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
  [../]
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

  [./Quadrature]
    type = TRAP
    order = FIRST
  [../]
[]

[Postprocessors]
  [./m_dot_inlet]
    type = RealComponentParameterValuePostprocessor
    component = inlet
    parameter = m_dot
  [../]
  [./T_inlet]
    type = RealComponentParameterValuePostprocessor
    component = inlet
    parameter = T
  [../]
[]

[Outputs]
  [./out]
    type = CSV
  [../]
[]
