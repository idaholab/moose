T_in = 300.         # K
m_dot_in = 1e-4     # kg/s
press = 1e5         # Pa

# core parameters
core_length = 1.    # m
core_n_elems = 10
core_dia = ${units 2. cm -> m}
core_pitch = ${units 8.7 cm -> m}

# pipe parameters
pipe_dia = ${units 10. cm -> m}

tot_power = 100     # W

# heat exchanger parameters
hx_dia_inner = ${units 10. cm -> m}
hx_wall_thickness = ${units 5. mm -> m}
hx_dia_outer = ${units 50. cm -> m}
hx_radius_wall = ${fparse hx_dia_inner / 2. + hx_wall_thickness}
hx_length = 1       # m
hx_n_elems = 10

m_dot_sec_in = 1    # kg/s

flow_blocks = 'core_chan up_pipe top_pipe hx/pri hx/sec down_pipe bottom_b bottom_a'
ht_blocks = 'core_chan hx/pri hx/sec'


[GlobalParams]
  initial_p = ${press}
  initial_vel = 0
  initial_T = ${T_in}
  initial_vel_x = 0
  initial_vel_y = 0
  initial_vel_z = 0

  rdg_slope_reconstruction = full
  closures = no_closures
  fp = he
[]

[Functions]
  [m_dot_sec_fn]
    type = PiecewiseLinear
    xy_data = '
      0    0
      100 ${m_dot_sec_in}'
  []
[]

[Materials]
  [f_mat]
    type = ADWallFrictionChurchillMaterial
    block = ${flow_blocks}
    D_h = D_h
    f_D = f_D
    mu = mu
    rho = rho
    vel = vel
  []

  [Hw_mat]
    type = ADWallHeatTransferCoefficient3EqnDittusBoelterMaterial
    block = ${ht_blocks}
    D_h = D_h
    rho = rho
    vel = vel
    T = T
    T_wall = T_wall
    cp = cp
    mu = mu
    k = k
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
  [no_closures]
    type = Closures1PhaseNone
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
  [total_power]
    type = TotalPower
    power = ${tot_power}
  []

  [core_chan]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '0 0 1'
    length = ${core_length}
    n_elems = ${core_n_elems}
    A = ${fparse core_pitch * core_pitch - pi * core_dia * core_dia / 4.}
    D_h = ${core_dia}
  []

  [core_hs]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '0 0 1'
    length = ${core_length}
    n_elems = ${core_n_elems}

    names = 'block'
    widths = '${fparse core_dia / 2.}'
    materials = 'steel'
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
    P_hf = ${fparse pi * core_dia}
  []

  [jct1]
    type = JunctionParallelChannels1Phase
    position = '0 0 1'
    connections = 'core_chan:out up_pipe:in'
    volume = 1e-3
  []

  [up_pipe]
    type = FlowChannel1Phase
    position = '0 0 1'
    orientation = '0 0 1'
    length = 1
    n_elems = 10
    A = ${fparse pi * pipe_dia * pipe_dia / 4.}
    D_h = ${pipe_dia}
  []

  [jct2]
    type = VolumeJunction1Phase
    position = '0 0 2'
    connections = 'up_pipe:out top_pipe:in'
    volume = 1e-3
  []

  [top_pipe]
    type = FlowChannel1Phase
    position = '0 0 2'
    orientation = '1 0 0'
    length = 1
    n_elems = 10
    A = ${fparse pi * pipe_dia * pipe_dia / 4.}
    D_h = ${pipe_dia}
  []

  [jct3]
    type = VolumeJunction1Phase
    position = '1 0 2'
    connections = 'top_pipe:out hx/pri:in'
    volume = 1e-3
  []

  [hx]
    [pri]
      type = FlowChannel1Phase
      position = '1 0 2'
      orientation = '0 0 -1'
      length = ${hx_length}
      n_elems = ${hx_n_elems}
      A = ${fparse pi * hx_dia_inner * hx_dia_inner / 4.}
      D_h = ${hx_dia_inner}
    []

    [ht_pri]
      type = HeatTransferFromHeatStructure1Phase
      hs = hx/wall
      hs_side = inner
      flow_channel = hx/pri
    []

    [wall]
      type = HeatStructureCylindrical
      position = '1 0 2'
      orientation = '0 0 -1'
      length = ${hx_length}
      n_elems = ${hx_n_elems}
      widths = '${hx_wall_thickness}'
      n_part_elems = '3'
      materials = 'steel'
      names = '0'
      inner_radius = ${fparse hx_dia_inner / 2.}
      offset_mesh_by_inner_radius = true
    []

    [ht_sec]
      type = HeatTransferFromHeatStructure1Phase
      hs = hx/wall
      hs_side = outer
      flow_channel = hx/sec
      P_hf = ${fparse 2 * pi * hx_radius_wall}
    []

    [sec]
      type = FlowChannel1Phase
      position = '${fparse 1 + hx_wall_thickness} 0 2'
      orientation = '0 0 -1'
      length = ${hx_length}
      n_elems = ${hx_n_elems}
      A = ${fparse pi * (hx_dia_outer * hx_dia_outer / 4. - hx_radius_wall * hx_radius_wall)}
      D_h = ${fparse hx_dia_outer - (2 * hx_radius_wall)}
      fp = water
    []
  []

  [jct4]
    type = VolumeJunction1Phase
    position = '1 0 1'
    connections = 'hx/pri:out down_pipe:in'
    volume = 1e-3
  []

  [down_pipe]
    type = FlowChannel1Phase
    position = '1 0 1'
    orientation = '0 0 -1'
    length = 1
    n_elems = 10
    A = ${fparse pi * pipe_dia * pipe_dia / 4.}
    D_h = ${pipe_dia}
  []

  [jct5]
    type = VolumeJunction1Phase
    position = '1 0 0'
    connections = 'down_pipe:out bottom_b:in'
    volume = 1e-3
  []

  [bottom_b]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '-1 0 0'
    length = 0.5
    n_elems = 5
    A = ${fparse pi * pipe_dia * pipe_dia / 4.}
    D_h = ${pipe_dia}
  []

  [pump]
    type = Pump1Phase
    position = '0.5 0 0'
    connections = 'bottom_b:out bottom_a:in'
    volume = 1e-3
    A_ref = ${fparse pi * pipe_dia * pipe_dia / 4.}
    head = 0
  []

  [bottom_a]
    type = FlowChannel1Phase
    position = '0.5 0 0'
    orientation = '-1 0 0'
    length = 0.5
    n_elems = 5
    A = ${fparse pi * pipe_dia * pipe_dia / 4.}
    D_h = ${pipe_dia}
  []

  [jct6]
    type = VolumeJunction1Phase
    position = '0 0 0'
    connections = 'bottom_a:out core_chan:in'
    volume = 1e-3
  []

  [inlet_sec]
    type = InletMassFlowRateTemperature1Phase
    input = 'hx/sec:out'
    m_dot = 0
    T = 300
  []

  [outlet_sec]
    type = Outlet1Phase
    input = 'hx/sec:in'
    p = ${press}
  []
[]

[ControlLogic]
  [set_point]
    type = GetFunctionValueControl
    function = ${m_dot_in}
  []

  [pid]
    type = PIDControl
    initial_value = 0
    set_point = set_point:value
    input = m_dot_pump
    K_p = 250
    K_i = 0.5
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
  [m_dot_pump]
    type = ADFlowJunctionFlux1Phase
    boundary = core_chan:in
    connection_index = 1
    equation = mass
    junction = jct6
  []

  [core_T_out]
    type = SideAverageValue
    boundary = core_chan:out
    variable = T
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
[]

[Executioner]
  type = Transient
  start_time = 0

  [TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 1
  []
  dtmax = 100
  end_time = 50000

  line_search = basic
  solve_type = NEWTON

  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-5
  nl_max_its = 5
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
