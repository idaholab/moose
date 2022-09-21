T_in = 300.         # K
m_dot_in = 1e-4     # kg/s
press = 1e5         # Pa

# core parameters
core_length = 1.    # m
core_n_elems = 10
core_dia = ${units 2. cm -> m}
core_pitch = ${units 8.7 cm -> m}

tot_power = 100       # W

[GlobalParams]
  initial_p = ${press}
  initial_vel = 0
  initial_T = ${T_in}

  rdg_slope_reconstruction = full
  closures = simple_closures
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

  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'core_chan:in'
    m_dot = ${m_dot_in}
    T = ${T_in}
  []

  [core_chan]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '0 0 1'
    length = ${core_length}
    n_elems = ${core_n_elems}
    A = ${fparse core_pitch * core_pitch - pi * core_dia * core_dia / 4.}
    D_h = ${fparse (4 * core_pitch * core_pitch - pi * core_dia * core_dia) / (4 * core_pitch + pi * core_dia)}
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

  [outlet]
    type = Outlet1Phase
    input = 'core_chan:out'
    p = ${press}
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
