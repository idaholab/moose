# This input file tests compatibility of JunctionParallelChannels1Phase and CaloricallyImperfectGas.
# Loss coefficient is applied in first junction.
# Expected pressure drop from form loss ~0.5*K*rho_in*vel_in^2=0.5*100*3.219603*1 = 160.9 Pa
# Pressure drop from averall flow area change ~ 21.9 Pa
# Expected pressure drop ~ 182.8 Pa

T_in = 523.0
vel = 1
p_out = 7e6

[GlobalParams]
  initial_p = ${p_out}
  initial_vel = ${vel}
  initial_T = ${T_in}
  gravity_vector = '0 0 0'
  closures = simple_closures
  n_elems = 3
  f = 0

  scaling_factor_1phase = '1 1 1e-5'
  scaling_factor_rhoV = '1e2'
  scaling_factor_rhowV = '1e-2'
  scaling_factor_rhoEV = '1e-5'
[]

[Functions]
  [e_fn]
    type = PiecewiseLinear
    x = '100   280 300 350 400 450 500 550 600 700 800 900 1000 1200 1400 1600 1800 2000 2200 2400 2600 2800 3000 5000'
    y = '783.9 2742.3 2958.6 3489.2 4012.7 4533.3 5053.8 5574 6095.1 7140.2 8192.9 9256.3 10333.6 12543.9 14836.6 17216.3 19688.4 22273.7 25018.3 28042.3 31544.2 35818.1 41256.5 100756.5'
    scale_factor = 1e3
  []

  [mu_fn]
    type = PiecewiseLinear
    x = '100   280 300 350 400 450 500 550 600 700 800 900 1000 1200 1400 1600 1800 2000 2200 2400 2600 2800 3000 5000'
    y = '85.42 85.42 89.53 99.44 108.9 117.98 126.73 135.2 143.43 159.25 174.36 188.9 202.96 229.88 255.5 280.05 303.67 326.45 344.97 366.49 387.87 409.48 431.86 431.86'
    scale_factor = 1e-7
  []

  [k_fn]
    type = PiecewiseLinear
    x = '100 280 300 350 400 450 500 550 600 700 800 900 1000 1200 1400 1600 1800 2000 2200 2400 2600 2800 3000 5000'
    y = '186.82 186.82 194.11 212.69 231.55 250.38 268.95 287.19 305.11 340.24 374.92 409.66 444.75 511.13 583.42 656.44 733.32 826.53 961.15 1180.38 1546.31 2135.49 3028.08 3028.08'
    scale_factor = 1e-3
  []
[]

[FluidProperties]
  [fp]
    type = CaloricallyImperfectGas
    molar_mass = 0.002
    e = e_fn
    k = k_fn
    mu = mu_fn
    min_temperature = 100
    max_temperature = 5000
    out_of_bound_error = false
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [inlet_bc]
    type = InletVelocityTemperature1Phase
    input = 'inlet:in'
    vel = ${vel}
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
    type = JunctionParallelChannels1Phase
    position = '0 0 10'
    initial_vel_x = 0
    initial_vel_y = 0
    initial_vel_z = ${vel}
    K = 100
    connections = 'inlet:out channel1:in channel2:in'
    volume = 1
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
    type = JunctionParallelChannels1Phase
    position = '0 0 0'
    initial_vel_x = 1
    initial_vel_y = 0
    initial_vel_z = ${vel}
    connections = 'channel1:out channel2:out outlet:in'
    volume = 1
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
  [p_in]
    type = SideAverageValue
    variable = p
    boundary = inlet:in
  []
  [p_out]
    type = SideAverageValue
    variable = p
    boundary = outlet:out
  []
  [Delta_p]
    type = DifferencePostprocessor
    value1 = p_out
    value2 = p_in
  []
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
  end_time = 20
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    optimal_iterations = 8
    iteration_window = 2
  []
  timestep_tolerance = 1e-6
  abort_on_solve_fail = true

  line_search = basic
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
    show = 'net_mass_flow_rate_domain net_mass_flow_rate_volume_junction Delta_p'
  []
[]
