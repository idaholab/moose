# Tests that the steady pressure rise across an abrupt area expansion modeled with
# JunctionParallelChannels1Phase matches the Borda-Carnot sudden-expansion model.
# The fluid is an ideal gas, and there is no friction or heat transfer.

m_dot = 2.5
A1 = 0.0062
A2 = 0.0364829
T_in = 648.3
p_out = 1.5e6
# Specific gas constant of hydrogen, R / M = 8.314 / 2.016e-3 [J/(kg-K)], for the initial velocities
R_specific = 4124.1

[GlobalParams]
  initial_T = ${T_in}
  initial_p = ${p_out}
  f = 0
  fp = fp
  closures = simple_closures
  scaling_factor_1phase = '1 1e-3 1e-6'
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    molar_mass = 2.016e-3
    gamma = 1.394
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe1:in'
    m_dot = ${m_dot}
    T = ${T_in}
  []
  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 2.5
    n_elems = 40
    A = ${A1}
    initial_vel = ${fparse m_dot / (p_out / (R_specific * T_in) * A1)}
  []
  [junction]
    type = JunctionParallelChannels1Phase
    connections = 'pipe1:out pipe2:in'
    position = '2.5 0 0'
    volume = 0.01
    initial_vel_x = ${fparse m_dot / (p_out / (R_specific * T_in) * A1)}
    initial_vel_y = 0
    initial_vel_z = 0
  []
  [pipe2]
    type = FlowChannel1Phase
    position = '2.5 0 0'
    orientation = '1 0 0'
    length = 2.5
    n_elems = 40
    A = ${A2}
    initial_vel = ${fparse m_dot / (p_out / (R_specific * T_in) * A2)}
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe2:out'
    p = ${p_out}
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
  end_time = 2
  solve_type = NEWTON
  line_search = basic
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 20
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-4
    growth_factor = 1.2
    optimal_iterations = 6
  []
[]

[Postprocessors]
  [p_in]
    type = SideAverageValue
    variable = p
    boundary = 'pipe1:out'
  []
  [p_out]
    type = SideAverageValue
    variable = p
    boundary = 'pipe2:in'
  []
  [rho_in]
    type = SideAverageValue
    variable = rho
    boundary = 'pipe1:out'
  []
  [m_dot_in]
    type = ADFlowBoundaryFlux1Phase
    boundary = 'inlet'
    equation = mass
  []
  [dp_junction]
    type = DifferencePostprocessor
    value1 = p_out
    value2 = p_in
  []
  # Borda-Carnot sudden-expansion pressure rise: rho1 u1^2 (A1/A2) (1 - A1/A2)
  [dp_borda_carnot]
    type = ParsedPostprocessor
    expression = 'rho_in * (m_dot_in / (rho_in * ${A1}))^2 * (${A1} / ${A2}) * (1 - ${A1} / ${A2})'
    pp_names = 'rho_in m_dot_in'
  []
  # Relative difference from the Borda-Carnot pressure rise; zero for an exact match
  [dp_diff]
    type = ParsedPostprocessor
    expression = 'dp_junction / dp_borda_carnot - 1'
    pp_names = 'dp_junction dp_borda_carnot'
    execute_on = 'FINAL'
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'FINAL'
    show = 'dp_diff'
  []
[]
