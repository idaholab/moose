T_in = 300. # K
m_dot_in = 1e-2 # kg/s
press = 10e5 # Pa

# core parameters
core_length = 1. # m
core_n_elems = 25
core_dia = '${units 2. cm -> m}'
core_pitch = '${units 8.7 cm -> m}'
A_core = '${fparse core_pitch^2 - 0.25 *pi * core_dia^2}'
P_wet_core = '${fparse 4*core_pitch + pi * core_dia}'
Dh_core = '${fparse 4 * A_core / P_wet_core}'

# pipe parameters
pipe_dia = '${units 10. cm -> m}'
A_pipe = '${fparse 0.25 * pi * pipe_dia^2}'

tot_power = 2000 # W

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
  fp = he
[]

[Functions]
  [m_dot_sec_fn]
    type = PiecewiseLinear
    xy_data = '
      0    0
      10 ${m_dot_sec_in}'
  []
[]

[FluidProperties]
  [he]
    type = IdealGasFluidProperties
    molar_mass = 4e-3
    gamma = 1.67
    k = 0.2556
    mu = 3.22639e-5
  []

  [water]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
  []
[]

[Closures]
  [thm_closures]
    type = Closures1PhaseTHM
  []
[]

[SolidProperties]
  [steel]
    type = ThermalFunctionSolidProperties
    rho = 8050
    k = 45
    cp = 466
  []
[]

[Components]
  [total_power]
    type = TotalPower
    power = ${tot_power}
  []
  [up_pipe_1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '0 0 1'
    length = 0.5
    n_elems = 15
    A = ${A_pipe}
    D_h = ${pipe_dia}
  []

  [jct1]
    type = JunctionParallelChannels1Phase
    position = '0 0 0.5'
    connections = 'up_pipe_1:out core_chan:in'
    volume = 1e-5
    use_scalar_variables = false
  []
  [core_chan]
    type = FlowChannel1Phase
    position = '0 0 0.5'
    orientation = '0 0 1'
    length = ${core_length}
    n_elems = ${core_n_elems}
    roughness = .0001
    A = ${A_core}
    D_h = ${Dh_core}
  []

  [core_hs]
    type = HeatStructureCylindrical
    position = '0 0 0.5'
    orientation = '0 0 1'
    length = ${core_length}
    n_elems = ${core_n_elems}
    names = 'block'
    widths = '${fparse core_dia / 2.}'
    solid_properties = 'steel'
    solid_properties_T_ref = '300'
    n_part_elems = 3
  []

  [core_heating]
    type = HeatSourceFromTotalPower
    hs = core_hs
    regions = block
    power = total_power
  []

  [core_ht]
    type = HeatTransferFromHeatStructure1Phase
    flow_channel = core_chan
    hs = core_hs
    hs_side = outer
    P_hf = '${fparse pi * core_dia}'
  []

  [jct2]
    type = JunctionParallelChannels1Phase
    position = '0 0 1.5'
    connections = 'core_chan:out up_pipe_2:in'
    volume = 1e-5
    use_scalar_variables = false
  []

  [up_pipe_2]
    type = FlowChannel1Phase
    position = '0 0 1.5'
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
    position = '0 0 2'
    orientation = '1 0 0'
    length = 0.5
    n_elems = 10
    A = ${A_pipe}
    D_h = ${pipe_dia}
  []

  [top_pipe_2]
    type = FlowChannel1Phase
    position = '0.5 0 2'
    orientation = '1 0 0'
    length = 0.5
    n_elems = 10
    A = ${A_pipe}
    D_h = ${pipe_dia}
  []

  [jct4]
    type = VolumeJunction1Phase
    position = '0.5 0 2'
    volume = 1e-5
    connections = 'top_pipe_1:out top_pipe_2:in press_pipe:in'
    use_scalar_variables = false
  []

  [press_pipe]
    type = FlowChannel1Phase
    position = '0.5 0 2'
    orientation = '0 1 0'
    length = 0.2
    n_elems = 5
    A = ${A_pipe}
    D_h = ${pipe_dia}
  []

  [pressurizer]
    type = InletStagnationPressureTemperature1Phase
    p0 = ${press}
    T0 = ${T_in}
    input = press_pipe:out
  []

  [jct5]
    type = JunctionOneToOne1Phase
    connections = 'top_pipe_2:out down_pipe_1:in'
  []

  [down_pipe_1]
    type = FlowChannel1Phase
    position = '1 0 2'
    orientation = '0 0 -1'
    length = 0.25
    A = ${A_pipe}
    n_elems = 5
  []

  [jct6]
    type = JunctionParallelChannels1Phase
    position = '1 0 1.75'
    connections = 'down_pipe_1:out hx/pri:in'
    volume = 1e-5
    use_scalar_variables = false
  []

  [hx]
    [pri]
      type = FlowChannel1Phase
      position = '1 0 1.75'
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
      position = '1 0 1.75'
      orientation = '0 0 -1'
      length = ${hx_length}
      n_elems = ${hx_n_elems}
      widths = '${hx_wall_thickness}'
      n_part_elems = '3'
      solid_properties = 'steel'
      solid_properties_T_ref = '300'
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
      position = '${fparse 1 + hx_wall_thickness} 0 0.25'
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
    position = '1 0 0.5'
    connections = 'hx/pri:out down_pipe_2:in'
    volume = 1e-5
    use_scalar_variables = false
  []

  [down_pipe_2]
    type = FlowChannel1Phase
    position = '1 0 0.25'
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
    position = '1 0 0'
    orientation = '-1 0 0'
    length = 0.5
    n_elems = 5
    A = ${A_pipe}
    D_h = ${pipe_dia}
  []

  [pump]
    type = Pump1Phase
    position = '0.5 0 0'
    connections = 'bottom_1:out bottom_2:in'
    volume = 1e-4
    A_ref = ${A_pipe}
    head = 0
    use_scalar_variables = false
  []

  [bottom_2]
    type = FlowChannel1Phase
    position = '0.5 0 0'
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
    m_dot = 0
    T = 300
  []

  [outlet_sec]
    type = Outlet1Phase
    input = 'hx/sec:out'
    p = 1e5
  []
[]

[ControlLogic]
  [set_point]
    type = GetFunctionValueControl
    function = ${m_dot_in}
  []

  [pid]
    type = PIDControl
    initial_value = 0.0
    set_point = set_point:value
    input = m_dot_pump
    K_p = 1.
    K_i = 4.
    K_d = 0
  []

  [set_pump_head]
    type = SetComponentRealValueControl
    component = pump
    parameter = head
    value = pid:output
  []

  [m_dot_sec_inlet_ctrl]
    type = GetFunctionValueControl
    function = m_dot_sec_fn
  []

  [set_m_dot_sec_ctrl]
    type = SetComponentRealValueControl
    component = inlet_sec
    parameter = m_dot
    value = m_dot_sec_inlet_ctrl:value
  []
[]

[Postprocessors]
  [power_to_coolant]
    type = ADHeatRateConvection1Phase
    block = core_chan
    P_hf = '${fparse pi *core_dia}'
  []

  [m_dot_pump]
    type = ADFlowJunctionFlux1Phase
    boundary = core_chan:in
    connection_index = 1
    equation = mass
    junction = jct7
  []

  [core_T_out]
    type = SideAverageValue
    boundary = core_chan:out
    variable = T
  []

  [core_p_in]
    type = SideAverageValue
    boundary = core_chan:in
    variable = p
  []

  [core_p_out]
    type = SideAverageValue
    boundary = core_chan:out
    variable = p
  []

  [core_delta_p]
    type = ParsedPostprocessor
    pp_names = 'core_p_in core_p_out'
    expression = 'core_p_in - core_p_out'
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
    dt = 1
  []
  dtmax = 5
  end_time = 500

  line_search = basic
  solve_type = NEWTON

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_rel_tol = 0
  nl_abs_tol = 1e-8
  nl_max_its = 25

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
