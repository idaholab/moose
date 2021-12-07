# This input file tests mass conservation at steady-state by looking at the
# net mass flux into the domain.

T_in = 523.0
m_dot = 100
p_out = 7e6

[GlobalParams]
  initial_p = ${p_out}
  initial_vel = 1
  initial_T = ${T_in}
  gravity_vector = '0 0 0'
  closures = simple_closures
  n_elems = 3
  f = 0
  scaling_factor_1phase = '1 1 1e-5'
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [inlet_bc]
    type = InletMassFlowRateTemperature1Phase
    input = 'inlet:in'
    m_dot = ${m_dot}
    T = ${T_in}
  []
  [inlet]
    type = FlowChannel1Phase
    fp = fp
    position = '0 0 11'
    orientation = '0 0 -1'
    length = 1
    A = 3
  []
  [inlet_plenum]
    type = VolumeJunction1Phase
    position = '0 0 10'
    initial_vel_x = 0
    initial_vel_y = 0
    initial_vel_z = 1
    connections = 'inlet:out channel1:in channel2:in'
    volume = 1
    scaling_factor_rhoEV = '1e-5'
  []
  [channel1]
    type = FlowChannel1Phase
    fp = fp
    position = '0 0 10'
    orientation = '0 0 -1'
    length = 10
    A = 4
    D_h = 1
  []
  [K_bypass]
    type = FormLossFromFunction1Phase
    K_prime = 500
    flow_channel = channel1
  []
  [channel2]
    type = FlowChannel1Phase
    fp = fp
    position = '0 0 10'
    orientation = '0 0 -1'
    length = 10
    A = 1
    D_h = 1
  []
  [outlet_plenum]
    type = VolumeJunction1Phase
    position = '0 0 0'
    initial_vel_x = 1
    initial_vel_y = 0
    initial_vel_z = 1
    connections = 'channel1:out channel2:out outlet:in'
    volume = 1
    scaling_factor_rhoEV = '1e-5'
  []
  [outlet]
    type = FlowChannel1Phase
    fp = fp
    position = '0 0 0'
    orientation = '0 0 -1'
    length = 1
    A = 1
  []
  [outlet_bc]
    type = Outlet1Phase
    p = ${p_out}
    input = 'outlet:out'
  []
[]

[Postprocessors]
  [inlet_in_m_dot]
    type = ADFlowBoundaryFlux1Phase
    boundary = 'inlet_bc'
    equation = mass
  []
  [inlet_out_m_dot]
    type = ADFlowJunctionFlux1Phase
    boundary = 'inlet:out'
    connection_index = 0
    junction = inlet_plenum
    equation = mass
  []

  [channel1_in_m_dot]
    type = ADFlowJunctionFlux1Phase
    boundary = 'channel1:in'
    connection_index = 1
    junction = inlet_plenum
    equation = mass
  []
  [channel1_out_m_dot]
    type = ADFlowJunctionFlux1Phase
    boundary = 'channel1:out'
    connection_index = 0
    junction = outlet_plenum
    equation = mass
  []

  [channel2_in_m_dot]
    type = ADFlowJunctionFlux1Phase
    boundary = 'channel2:in'
    connection_index = 2
    junction = inlet_plenum
    equation = mass
  []
  [channel2_out_m_dot]
    type = ADFlowJunctionFlux1Phase
    boundary = 'channel2:out'
    connection_index = 1
    junction = outlet_plenum
    equation = mass
  []

  [outlet_in_m_dot]
    type = ADFlowJunctionFlux1Phase
    boundary = 'outlet:in'
    connection_index = 2
    junction = outlet_plenum
    equation = mass
  []
  [outlet_out_m_dot]
    type = ADFlowBoundaryFlux1Phase
    boundary = 'outlet_bc'
    equation = mass
  []

  [net_mass_flow_rate_domain]
    type = LinearCombinationPostprocessor
    pp_names = 'inlet_in_m_dot outlet_out_m_dot'
    pp_coefs = '1 -1'
  []
  [net_mass_flow_rate_volume_junction]
    type = LinearCombinationPostprocessor
    pp_names = 'inlet_out_m_dot channel1_in_m_dot channel2_in_m_dot'
    pp_coefs = '1 -1 -1'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  start_time = 0
  end_time = 10000
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.01
    optimal_iterations = 8
    iteration_window = 2
  []
  timestep_tolerance = 1e-6
  abort_on_solve_fail = true

  line_search = none
  nl_rel_tol = 1e-8
  nl_abs_tol = 2e-8
  nl_max_its = 25
  l_tol = 1e-3
  l_max_its = 5
  petsc_options = '-snes_converged_reason'
  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu     '
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'FINAL'
    show = 'net_mass_flow_rate_domain net_mass_flow_rate_volume_junction'
  []
[]
