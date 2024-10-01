[GlobalParams]
  scaling_factor_1phase = '1. 1.e-2 1.e-4'
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

[Components]
  [pipe1]
    type = FlowChannel1Phase
    fp = fp
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 3
    A = 1.907720E-04
    D_h = 1.698566E-02
    f = 0.1
  []

  [junction]
    type = VolumeJunction1Phase
    connections = 'pipe1:out pipe2:in'
    volume = 1
    position = '1 0 0'

    scaling_factor_rhoV  = 1
    scaling_factor_rhouV = 1
    scaling_factor_rhovV = 1
    scaling_factor_rhowV = 1
    scaling_factor_rhoEV = 1e-4

    use_scalar_variables = false
  []

  [pipe2]
    type = FlowChannel1Phase
    fp = fp
    # geometry
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 3
    A = 1.907720E-04
    D_h = 1.698566E-02
    f = 0.1
  []

  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe1:in'
    m_dot = 0.1
    T = 500
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe2:out'
    p = 6e6
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
  scheme = 'bdf2'

  start_time = 0
  dt = 1
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  nl_rel_tol = 0
  nl_abs_tol = 1e-10
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 100

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
  velocity_as_vector = false
[]
