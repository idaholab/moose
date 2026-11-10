# Tests that mass is conserved across a VolumeJunction1Phase when the area of
# the connected flow channels varies within the elements touching the junction.
# Both channels taper linearly to A_j at the junction, so the element-average
# area of each connected element differs from the area at the connection.

A1 = 0.0567
A2 = 0.02222
A_j = 0.03192
m_dot = 20

[GlobalParams]
  initial_T = 393.15
  initial_p = 1.318964e+07
  initial_vel = 0.5
  f = 0
  fp = fp
  closures = simple_closures
  scaling_factor_1phase = '1 1 1e-5'
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
  [A1_fn]
    type = PiecewiseLinear
    axis = x
    x = '0 0.5 1'
    y = '${A1} ${A1} ${A_j}'
  []
  [A2_fn]
    type = PiecewiseLinear
    axis = x
    x = '1 1.5 2'
    y = '${A_j} ${A2} ${A2}'
  []
[]

[Components]
  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe1:in'
    m_dot = ${m_dot}
    T = 393.15
  []
  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 20
    A = A1_fn
  []
  [junction]
    type = VolumeJunction1Phase
    connections = 'pipe1:out pipe2:in'
    position = '1 0 0'
    volume = 0.1
    scaling_factor_rhoEV = 1e-5
    initial_vel_x = 0.5
    initial_vel_y = 0
    initial_vel_z = 0
  []
  [pipe2]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 20
    A = A2_fn
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe2:out'
    p = 1.318964e+07
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
  dt = 0.2
  end_time = 40
  solve_type = NEWTON
  line_search = basic
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  nl_max_its = 20
  abort_on_solve_fail = true
[]

[Postprocessors]
  [m_dot_in]
    type = ADFlowBoundaryFlux1Phase
    boundary = 'inlet'
    equation = mass
    execute_on = 'FINAL'
  []
  [m_dot_junction_in]
    type = ADFlowJunctionFlux1Phase
    boundary = 'pipe1:out'
    connection_index = 0
    junction = junction
    equation = mass
    execute_on = 'FINAL'
  []
  [m_dot_junction_out]
    type = ADFlowJunctionFlux1Phase
    boundary = 'pipe2:in'
    connection_index = 1
    junction = junction
    equation = mass
    execute_on = 'FINAL'
  []
  [m_dot_out]
    type = ADFlowBoundaryFlux1Phase
    boundary = 'outlet'
    equation = mass
    execute_on = 'FINAL'
  []
  # Relative differences from the inlet mass flow rate; zero when mass is conserved
  [m_dot_junction_in_diff]
    type = ParsedPostprocessor
    expression = '(m_dot_junction_in - m_dot_in) / m_dot_in'
    pp_names = 'm_dot_junction_in m_dot_in'
    execute_on = 'FINAL'
  []
  [m_dot_junction_out_diff]
    type = ParsedPostprocessor
    expression = '(m_dot_junction_out - m_dot_in) / m_dot_in'
    pp_names = 'm_dot_junction_out m_dot_in'
    execute_on = 'FINAL'
  []
  [m_dot_out_diff]
    type = ParsedPostprocessor
    expression = '(m_dot_out - m_dot_in) / m_dot_in'
    pp_names = 'm_dot_out m_dot_in'
    execute_on = 'FINAL'
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'FINAL'
    show = 'm_dot_junction_in_diff m_dot_junction_out_diff m_dot_out_diff'
  []
[]
