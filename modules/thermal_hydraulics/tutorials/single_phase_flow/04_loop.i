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

[GlobalParams]
  initial_p = ${press}
  initial_vel = 0
  initial_T = ${T_in}
  initial_vel_x = 0
  initial_vel_y = 0
  initial_vel_z = 0

  rdg_slope_reconstruction = full
  closures = simple_closures
  fp = he
  f = 0.4
[]

[FluidProperties]
  [he]
    type = IdealGasFluidProperties
    molar_mass = 4e-3
    gamma = 1.67
    k = 0.2556
    mu = 3.22639e-5
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
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
    f = 1.6
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
    Hw = 1.36
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
    connections = 'top_pipe:out cooling_pipe:in'
    volume = 1e-3
  []

  [cooling_pipe]
    type = FlowChannel1Phase
    position = '1 0 2'
    orientation = '0 0 -1'
    length = 1
    n_elems = 10
    A = ${fparse pi * pipe_dia * pipe_dia / 4.}
    D_h = ${pipe_dia}
  []

  [cold_wall]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = cooling_pipe
    T_wall = 300
    Hw = 0.97
  []

  [jct4]
    type = VolumeJunction1Phase
    position = '1 0 1'
    connections = 'cooling_pipe:out down_pipe:in'
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
    boundary = cooling_pipe:out
    variable = T
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 1000
  dt = 10

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
