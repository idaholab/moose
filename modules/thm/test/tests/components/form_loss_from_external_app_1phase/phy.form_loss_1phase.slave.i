[GlobalParams]
  initial_p = 1e5
  initial_vel = 0.5
  initial_T = 300.0

  gravity_vector = '0 0 0'

  scaling_factor_1phase = '1 1e-2 1e-4'

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
  [pipe]
    type = FlowChannel1Phase
    fp = fp
    position = '0 0 0'
    orientation = '1 0 0'
    length = 2
    A = 1
    n_elems = 10

    f = 0
  []
  [form_loss]
    type = FormLossFromExternalApp1Phase
    flow_channel = pipe
  []
  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe:in'
    m_dot = 680
    T = 300
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 1e5
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  dt = 0.1
  abort_on_solve_fail = true
  timestep_tolerance = 5e-14

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 1e-8
  nl_abs_tol = 5e-8
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 20

  start_time = 0.0
  end_time = 4.0

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Outputs]
  exodus = true
  show = 'K_prime p'
[]
