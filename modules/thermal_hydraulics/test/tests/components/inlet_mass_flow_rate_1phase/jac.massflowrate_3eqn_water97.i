[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T = 444.447
  initial_p = 7e6
  initial_vel = 0

  closures = simple_closures
[]

[FluidProperties]
  [fp]
    type = Water97FluidProperties
    T_initial_guess = 444.447
    p_initial_guess = 7e6
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
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    A   = 1.0000000000e-04
    D_h  = 1.1283791671e-02
    f = 0.1
    length = 1
    n_elems = 4
  []

  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe:in'
    m_dot = 0.18
    T     = 444.447
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 7e6
  []
[]

[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  dt = 0.1
  start_time = 0.0
  num_steps = 30

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 0
  nl_abs_tol = 1e-6
  nl_max_its = 20

  l_tol = 1e-3
  l_max_its = 100

  abort_on_solve_fail = true

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Outputs]
  exodus = true
[]
