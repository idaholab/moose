# THM file based on https://mooseframework.inl.gov/modules/thermal_hydraulics/tutorials/single_phase_flow/step05.html
# Used to couple THM with SC
# This is a simple closed loop with a pump providing pressure head, core, pressurizer and HX.
# THM sends massflux and temperature at the inlet of the core, and pressure at the outlet of the core
# to subchannel. Subchannel returns total pressure drop of the assembly and total power to THM and THM calculates an
# average friction factor for the core region.
T_in = 583.0 # K
press = 2e5 # Pa
SC_core = 0.0004980799633447909 #m2

# core parameters
core_length = 1. # m
core_n_elems = 1
A_core = 0.005 #dummy

# pipe parameters
pipe_dia = '${units 10. cm -> m}'
A_pipe = '${fparse 0.25 * pi * pipe_dia^2}'

# heat exchanger parameters
hx_dia_inner = '${units 12. cm -> m}'
hx_wall_thickness = '${units 5. mm -> m}'
hx_dia_outer = '${units 50. cm -> m}'
hx_radius_wall = '${fparse hx_dia_inner / 2. + hx_wall_thickness}'
hx_length = 1.5 # m
hx_n_elems = 25

m_dot_sec_in = 1. # kg/s

[GlobalParams]
  initial_p = ${press}
  initial_vel = 0.0001
  initial_T = ${T_in}
  initial_vel_x = 0
  initial_vel_y = 0
  initial_vel_z = 0
  gravity_vector = '0 0 0'

  rdg_slope_reconstruction = minmod
  scaling_factor_1phase = '1 1e-2 1e-4'
  scaling_factor_rhoV = 1
  scaling_factor_rhouV = 1e-2
  scaling_factor_rhovV = 1e-2
  scaling_factor_rhowV = 1e-2
  scaling_factor_rhoEV = 1e-4
  closures = thm_closures
  fp = sodium_eos
[]

[Functions]
  [q_wall_fn]
    type = ParsedFunction
    symbol_names = 'core_power length'
    symbol_values = 'core_power  ${core_length}'
    expression = 'core_power/length'
  []
[]

[FluidProperties]
  [water]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
  []

  [sodium_eos]
    type = StiffenedGasFluidProperties
    gamma = 1.24
    cv = 1052.8
    q = -2.6292e+05
    p_inf = 1.1564e+08
    q_prime = 0
    mu = 3.222e-04
    k = 73.82
  []
[]

[Closures]
  [thm_closures]
    type = Closures1PhaseTHM
  []
  [none_closures]
    type = Closures1PhaseNone
  []
[]

[Materials]
  [f_mat]
    type = ADParsedMaterial
    property_name = f_D
    postprocessor_names = 'core_f'
    expression = 'core_f'
    block = 'core_chan'
  []
[]

[HeatStructureMaterials]
  [steel]
    type = SolidMaterialProperties
    rho = 8050
    k = 45
    cp = 466
  []
[]

[Components]
  [up_pipe_1]
    type = FlowChannel1Phase
    position = '0 0 -0.5'
    orientation = '0 0 1'
    length = 0.5
    n_elems = 15
    A = ${A_pipe}
    D_h = ${pipe_dia}
  []

  [jct1]
    type = JunctionParallelChannels1Phase
    position = '0 0 0'
    connections = 'up_pipe_1:out core_chan:in'
    volume = 1e-5
  []

  [core_chan]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '0 0 1'
    length = ${core_length}
    n_elems = ${core_n_elems}
    A = ${A_core}
    closures = none_closures
  []

  [core_ht]
    type = HeatTransferFromHeatFlux1Phase
    flow_channel = core_chan
    q_wall = q_wall_fn
    P_hf = 1
  []

  [jct2]
    type = JunctionParallelChannels1Phase
    position = '0 0 1'
    connections = 'core_chan:out up_pipe_2:in'
    volume = 1e-5
  []

  [up_pipe_2]
    type = FlowChannel1Phase
    position = '0 0 1'
    orientation = '0 0 1'
    length = 0.5
    n_elems = 10
    A = ${A_pipe}
    D_h = ${pipe_dia}
  []

  [jct3]
    type = JunctionOneToOne1Phase
    connections = 'up_pipe_2:out top_pipe_1:in'
  []

  [top_pipe_1]
    type = FlowChannel1Phase
    position = '0 0 1.5'
    orientation = '1 0 0'
    length = 0.5
    n_elems = 10
    A = ${A_pipe}
    D_h = ${pipe_dia}
  []
  [top_pipe_2]
    type = FlowChannel1Phase
    position = '0.5 0 1.5'
    orientation = '1 0 0'
    length = 0.5
    n_elems = 10
    A = ${A_pipe}
    D_h = ${pipe_dia}
  []

  [jct4]
    type = VolumeJunction1Phase
    position = '0.5 0 1.5'
    volume = 1e-5
    connections = 'top_pipe_1:out top_pipe_2:in press_pipe:in'
  []

  [press_pipe]
    type = FlowChannel1Phase
    position = '0.5 0 1.5'
    orientation = '0 1 0'
    length = 0.2
    n_elems = 5
    A = ${A_pipe}
    D_h = ${pipe_dia}
  []

  [pressurizer]
    type = InletStagnationPressureTemperature1Phase
    p0 = ${press}
    T0 = 580
    input = press_pipe:out
  []

  [jct5]
    type = JunctionOneToOne1Phase
    connections = 'top_pipe_2:out down_pipe_1:in'
  []

  [down_pipe_1]
    type = FlowChannel1Phase
    position = '1 0 1.5'
    orientation = '0 0 -1'
    length = 0.25
    A = ${A_pipe}
    n_elems = 5
  []

  [jct6]
    type = JunctionParallelChannels1Phase
    position = '1 0 1.25'
    connections = 'down_pipe_1:out hx/pri:in'
    volume = 1e-5
  []

  [hx]
    [pri]
      type = FlowChannel1Phase
      position = '1 0 1.25'
      orientation = '0 0 -1'
      length = ${hx_length}
      n_elems = ${hx_n_elems}
      roughness = 1e-5
      A = '${fparse pi * hx_dia_inner * hx_dia_inner / 4.}'
      D_h = ${hx_dia_inner}
    []

    [ht_pri]
      type = HeatTransferFromHeatStructure1Phase
      hs = hx/wall
      hs_side = inner
      flow_channel = hx/pri
      P_hf = '${fparse pi * hx_dia_inner}'
    []

    [wall]
      type = HeatStructureCylindrical
      position = '1 0 1.25'
      orientation = '0 0 -1'
      length = ${hx_length}
      n_elems = ${hx_n_elems}
      widths = '${hx_wall_thickness}'
      n_part_elems = '3'
      materials = 'steel'
      names = '0'
      inner_radius = '${fparse hx_dia_inner / 2.}'
    []

    [ht_sec]
      type = HeatTransferFromHeatStructure1Phase
      hs = hx/wall
      hs_side = outer
      flow_channel = hx/sec
      P_hf = '${fparse 2 * pi * hx_radius_wall}'
    []

    [sec]
      type = FlowChannel1Phase
      position = '${fparse 1 + hx_wall_thickness} 0 -0.25'
      orientation = '0 0 1'
      length = ${hx_length}
      n_elems = ${hx_n_elems}
      A = '${fparse pi * (hx_dia_outer * hx_dia_outer / 4. - hx_radius_wall * hx_radius_wall)}'
      D_h = '${fparse hx_dia_outer - (2 * hx_radius_wall)}'
      fp = water
      initial_T = 300
    []
  []

  [jct7]
    type = JunctionParallelChannels1Phase
    position = '1 0 -0.25'
    connections = 'hx/pri:out down_pipe_2:in'
    volume = 1e-5
  []

  [down_pipe_2]
    type = FlowChannel1Phase
    position = '1 0 -0.25'
    orientation = '0 0 -1'
    length = 0.25
    n_elems = 10
    A = ${A_pipe}
    D_h = ${pipe_dia}
  []

  [jct8]
    type = JunctionOneToOne1Phase
    connections = 'down_pipe_2:out bottom_1:in'
  []

  [bottom_1]
    type = FlowChannel1Phase
    position = '1 0 -0.5'
    orientation = '-1 0 0'
    length = 0.5
    n_elems = 5
    A = ${A_pipe}
    D_h = ${pipe_dia}
  []

  [pump]
    type = Pump1Phase
    position = '0.5 0 -0.5'
    connections = 'bottom_1:out bottom_2:in'
    volume = 1e-4
    A_ref = ${A_pipe}
    head = 3.56
  []

  [bottom_2]
    type = FlowChannel1Phase
    position = '0.5 0 -0.5'
    orientation = '-1 0 0'
    length = 0.5
    n_elems = 5
    A = ${A_pipe}
    D_h = ${pipe_dia}
  []

  [jct9]
    type = JunctionOneToOne1Phase
    connections = 'bottom_2:out up_pipe_1:in'
  []

  [inlet_sec]
    type = InletMassFlowRateTemperature1Phase
    input = 'hx/sec:in'
    m_dot = ${m_dot_sec_in}
    T = 300
  []

  [outlet_sec]
    type = Outlet1Phase
    input = 'hx/sec:out'
    p = 1e5
  []
[]

[Postprocessors]
  [power_to_coolant]
    type = ADHeatRateDirectFlowChannel
    q_wall_prop = q_wall
    block = core_chan
    P_hf = 1
  []

  [core_T_out]
    type = SideAverageValue
    boundary = core_chan:out
    variable = T
  []

  [T_out]
    type = SideAverageValue
    boundary = bottom_1:out
    variable = T
  []

  [core_p_in]
    type = SideAverageValue
    boundary = up_pipe_1:out
    variable = p
  []

  [core_p_out]
    type = SideAverageValue
    boundary = up_pipe_2:in
    variable = p
  []

  [core_delta_p]
    type = ParsedPostprocessor
    pp_names = 'core_p_in core_p_out'
    function = 'core_p_in - core_p_out'
  []

  [hx_pri_T_out]
    type = SideAverageValue
    boundary = hx/pri:out
    variable = T
  []

  [hx_sec_T_in]
    type = SideAverageValue
    boundary = inlet_sec
    variable = T
  []

  [hx_sec_T_out]
    type = SideAverageValue
    boundary = outlet_sec
    variable = T
  []

  [m_dot_sec]
    type = ADFlowBoundaryFlux1Phase
    boundary = inlet_sec
    equation = mass
  []
  ############## Friction Factor Calculation #############
  [av_rhouA]
    type = ElementAverageValue
    variable = 'rhouA'
    block = 'core_chan'
  []

  [av_rho]
    type = ElementAverageValue
    variable = 'rho'
    block = 'core_chan'
  []

  [Kloss]
    type = ParsedPostprocessor
    pp_names = 'core_delta_p_tgt av_rhouA av_rho'
    function = '2.0 * core_delta_p_tgt * av_rho * ${A_core} * ${A_core} / (av_rhouA * av_rhouA)'
  []

  [Dh]
    type = ADElementAverageMaterialProperty
    mat_prop = D_h
    block = core_chan
  []

  [core_f]
    type = ParsedPostprocessor
    pp_names = 'Kloss Dh'
    function = 'Kloss * Dh / ${core_length}'
  []

  ### INFO to send to SC
  [outlet_pressure]
    type = SideAverageValue
    boundary = up_pipe_2:in
    variable = p
  []

  [inlet_mass_flow_rate]
    type = ADFlowJunctionFlux1Phase
    boundary = up_pipe_1:out
    connection_index = 0
    equation = mass
    junction = jct1
  []

  [inlet_temperature]
    type = SideAverageValue
    boundary = up_pipe_1:out
    variable = T
  []

  [inlet_mass_flux]
    type = ParsedPostprocessor
    pp_names = 'inlet_mass_flow_rate'
    function = 'abs(inlet_mass_flow_rate/${SC_core})'
  []
  #####
  ##### Info received from subchannel
  [core_delta_p_tgt]
    type = Receiver
    default = 100
  []
  [core_power]
    type = Receiver
    default = 100
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
  start_time = 0

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 2
  []
  dtmax = 50
  end_time = 3000

  line_search = basic
  solve_type = NEWTON

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_rel_tol = 1e-7
  nl_abs_tol = 1e-7
  nl_max_its = 25

  fixed_point_min_its = 1
  fixed_point_max_its = 5
  accept_on_max_fixed_point_iteration = true
  auto_advance = true
  relaxation_factor = 0.5
[]

[Outputs]
  exodus = true

  [console]
    type = Console
    max_rows = 1
    outlier_variable_norms = false
  []
  print_linear_residuals = false
[]

################################################################################
# A multiapp that couples THM to subchannel
################################################################################

[MultiApps]
  # active = ''
  [subchannel]
    type = FullSolveMultiApp
    input_files = 'subchannel.i'
    execute_on = 'timestep_end'
    positions = '0 0 0'
    max_procs_per_app = 1
    output_in_position = true
    bounding_box_padding = '0 0 0.1'
  []
[]

[Transfers]
  # active = ''
  [pressure_drop_transfer] # Get pressure drop to THM from subchannel
    type = MultiAppPostprocessorTransfer
    from_multi_app = subchannel
    from_postprocessor = total_pressure_drop_SC
    to_postprocessor = core_delta_p_tgt
    reduction_type = average
    execute_on = 'timestep_end'
  []

  [power_transfer] # Get Total power to THM from subchannel
    type = MultiAppPostprocessorTransfer
    from_multi_app = subchannel
    from_postprocessor = Total_power
    to_postprocessor = core_power
    reduction_type = average
    execute_on = 'timestep_end'
  []

  [mass_flux_tranfer] # Send mass_flux at the inlet of THM core to subchannel
    type = MultiAppPostprocessorTransfer
    to_multi_app = subchannel
    from_postprocessor = inlet_mass_flux
    to_postprocessor = report_mass_flux_inlet
    execute_on = 'timestep_end'
  []

  [outlet_pressure_tranfer] # Send pressure at the outlet of THM core to subchannel
    type = MultiAppPostprocessorTransfer
    to_multi_app = subchannel
    from_postprocessor = outlet_pressure
    to_postprocessor = report_pressure_outlet
    execute_on = 'timestep_end'
  []

  [inlet_temperature_transfer]
    type = MultiAppPostprocessorTransfer
    to_multi_app = subchannel
    from_postprocessor = inlet_temperature
    to_postprocessor = report_temperature_inlet
    execute_on = 'timestep_end'
  []
[]
