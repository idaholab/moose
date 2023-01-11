[GlobalParams]
  scaling_factor_1phase = '1. 1.e-2 1.e-4'
  scaling_factor_temperature = 1e-2

  initial_T = 500
  initial_p = 6.e6
  initial_vel = 0

  closures = simple_closures
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
    k = 0.5
    mu = 281.8e-6
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Materials]
  [mat]
    type = ADGenericConstantMaterial
    prop_names = 'density specific_heat thermal_conductivity'
    prop_values = '16 356 6.5514e3'
  []
[]

[Functions]
  [Ts_init]
    type = ParsedFunction
    expression = '2*sin(x*pi/2)+2*sin(pi*y) +507'
  []
[]

[Components]
  [pipe]
    type = FlowChannel1Phase
    fp = fp
    # geometry
    position = '-1 0 -2.5'
    orientation = '1 0 0'
    length = 2
    n_elems = 2
    A = 0.3
    D_h = 0.1935483871
    f = 0.1
  []
  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe:in'
    m_dot = 0.1
    T = 500
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 6e6
  []
  [ht]
    type = HeatTransferFromHeatStructure3D1Phase
    flow_channels = 'pipe'
    hs = blk
    boundary = blk:right
    P_hf = 3
    Hw = 1000
  []
  [blk]
    type = HeatStructureFromFile3D
    file = box.e
    position = '0 0 0'
    initial_T = Ts_init
  []
  [right_bnd]
    type = HSBoundarySpecifiedTemperature
    hs = blk
    boundary = blk:bottom
    T = Ts_init
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 100
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 1e-7
  nl_abs_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 100

  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu'
[]

[Outputs]
  exodus = true
  execute_on = 'initial final'
[]
