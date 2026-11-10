# Tests that the energy added by the pump equals the work of an ideal pump,
# m_dot * g * head, so that the total enthalpy rises by g * head across the pump.
# Friction is zero and the flow channels are horizontal, so the pump is the only
# source of energy.

head = 95.
g = 9.81
volume = 0.1
m_dot = 20
A = 0.0567
rho_initial = 940

[GlobalParams]
  initial_T = 393.15
  initial_p = 1.318964e+07
  A = ${A}
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
    initial_vel = ${fparse m_dot / (rho_initial * A)}
  []
  [pump]
    type = Pump1Phase
    connections = 'pipe1:out pipe2:in'
    position = '1 0 0'
    head = ${head}
    volume = ${volume}
    A_ref = ${A}
    scaling_factor_rhoEV = 1e-5
    initial_vel_x = ${fparse m_dot / (rho_initial * A)}
    initial_vel_y = 0
    initial_vel_z = 0
  []
  [pipe2]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 20
    initial_vel = ${fparse m_dot / (rho_initial * A)}
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
  end_time = 60
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
  [E_flux_in]
    type = ADFlowBoundaryFlux1Phase
    boundary = 'inlet'
    equation = energy
    execute_on = 'FINAL'
  []
  [E_flux_out]
    type = ADFlowBoundaryFlux1Phase
    boundary = 'outlet'
    equation = energy
    execute_on = 'FINAL'
  []
  # Energy added per unit mass, relative to the ideal pump work g * head; zero for an ideal pump
  [energy_input_diff]
    type = ParsedPostprocessor
    expression = '(E_flux_out - E_flux_in) / m_dot_in / (${g} * ${head}) - 1'
    pp_names = 'E_flux_out E_flux_in m_dot_in'
    execute_on = 'FINAL'
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'FINAL'
    show = 'energy_input_diff'
  []
[]
