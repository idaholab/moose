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

tot_power = 2000 # W

[GlobalParams]
  initial_p = ${press}
  initial_vel = 0.0001
  initial_T = ${T_in}
  gravity_vector = '0 0 0'

  rdg_slope_reconstruction = minmod
  scaling_factor_1phase = '1 1e-2 1e-4'
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
    roughness = .0001
    A = '${A_core}'
    D_h = ${Dh_core}
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
    P_hf = '${fparse pi * core_dia}'
  []

  [outlet]
    type = Outlet1Phase
    input = 'core_chan:out'
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
    function = 'core_p_in - core_p_out'
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
    dt = 10
  []
  end_time = 5000

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
