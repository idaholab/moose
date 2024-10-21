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
  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'up_pipe_1:in'
    m_dot = ${m_dot_in}
    T = ${T_in}
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
    A = '${A_core}'
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
    connections = 'up_pipe_2:out top_pipe:in'
  []

  [top_pipe]
    type = FlowChannel1Phase
    position = '0 0 2'
    orientation = '1 0 0'
    length = 1
    n_elems = 10
    A = ${A_pipe}
    D_h = ${pipe_dia}
  []

  [jct4]
    type = JunctionOneToOne1Phase
    connections = 'top_pipe:out down_pipe_1:in'
  []

  [down_pipe_1]
    type = FlowChannel1Phase
    position = '1 0 2'
    orientation = '0 0 -1'
    length = 0.25
    A = ${A_pipe}
    n_elems = 5
  []

  [jct5]
    type = JunctionOneToOne1Phase
    connections = 'down_pipe_1:out cooling_pipe:in'
  []

  [cooling_pipe]
    type = FlowChannel1Phase
    position = '1 0 1.75'
    orientation = '0 0 -1'
    length = 1.5
    n_elems = 25
    A = ${A_pipe}
  []

  [cold_wall]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = cooling_pipe
    T_wall = 300
    P_hf = '${fparse pi * pipe_dia}'
  []

  [jct6]
    type = JunctionOneToOne1Phase
    connections = 'cooling_pipe:out down_pipe_2:in'
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

  [outlet]
    type = Outlet1Phase
    input = 'down_pipe_2:out'
    p = ${press}
  []
[]

[Postprocessors]
  [power_to_coolant]
    type = ADHeatRateConvection1Phase
    block = core_chan
    P_hf = '${fparse pi *core_dia}'
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
    boundary = cooling_pipe:out
    variable = T
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
  end_time = 500

  line_search = basic
  solve_type = NEWTON

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_rel_tol = 1e-8
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
