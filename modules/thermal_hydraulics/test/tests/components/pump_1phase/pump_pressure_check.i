# This test checks that the expected pressure rise due to the user supplied
# pump head matches the actual pressure rise across the pump.
# The orientation of flow channels in this test have no components in the z-direction
# due to the expected_pressure_rise_fcn not accounting for hydrostatic pressure.

head = 95.
dt = 0.1
g = 9.81
volume = 0.567

[GlobalParams]
  initial_T = 393.15
  initial_vel = 0.0372
  A = 0.567
  f = 0
  fp = fp
  scaling_factor_1phase = '1 1 1e-5'
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
  [expected_pressure_rise_fcn]
    type = ParsedFunction
    expression = 'rhoV * g * head / volume'
    symbol_names = 'rhoV g head volume'
    symbol_values = 'pump_rhoV ${g} ${head} ${volume}'
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
    initial_p = 1.318964e+07
    n_elems = 10
  []

  [pump]
    type = Pump1Phase
    connections = 'pipe1:out pipe2:in'
    position = '1.02 0 0'
    initial_p = 1.318964e+07
    scaling_factor_rhoEV = 1e-5
    head = ${head}
    volume = ${volume}
    A_ref = 0.567
    initial_vel_x = 1
    initial_vel_y = 1
    initial_vel_z = 0
    use_scalar_variables = false
  []

  [pipe2]
    type = FlowChannel1Phase
    position = '1.04 0 0'
    orientation = '0 2 0'
    length = 0.96
    initial_p = 1.4072E+07
    n_elems = 10
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe2:out'
    p = 1.4072E+07
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
  scheme = 'implicit-euler'
  start_time = 0
  dt = ${dt}
  num_steps = 4
  abort_on_solve_fail = true
  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  nl_max_its = 15
  l_tol = 1e-4
  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Postprocessors]
  [pump_rhoV]
    type = ElementAverageValue
    variable = rhoV
    block = 'pump'
    execute_on = 'initial timestep_end'
  []
  [expected_pressure_rise]
    type = FunctionValuePostprocessor
    function = expected_pressure_rise_fcn
    indirect_dependencies = 'pump_rhoV'
    execute_on = 'initial timestep_end'
  []
  [p_inlet]
    type = SideAverageValue
    variable = p
    boundary = 'pipe1:out'
    execute_on = 'initial timestep_end'
  []
  [p_outlet]
    type = SideAverageValue
    variable = p
    boundary = 'pipe2:in'
    execute_on = 'initial timestep_end'
  []
  [actual_pressure_rise]
    type = DifferencePostprocessor
    value1 = p_outlet
    value2 = p_inlet
    execute_on = 'timestep_end'
  []
  [pressure_rise_diff]
    type = RelativeDifferencePostprocessor
    value1 = actual_pressure_rise
    value2 = expected_pressure_rise
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'FINAL'
    show = 'pressure_rise_diff'
  []
[]
