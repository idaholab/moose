T_in = 300.         # K
m_dot_in = 1e-4     # kg/s
press = 1e5         # Pa

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

[Components]
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
    length = 1
    n_elems = 10
    A = 7.2548e-3
    D_h = 7.0636e-2
    f = 1.6
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
  end_time = 2
  dt = 0.1

  line_search = basic
  solve_type = NEWTON

  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-5
  nl_max_its = 5
[]

[Outputs]
  exodus = true
[]
