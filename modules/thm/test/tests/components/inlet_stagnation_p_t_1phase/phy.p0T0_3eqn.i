[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 1e6
  initial_T = 453.1
  initial_vel = 0.0

  scaling_factor_1phase = '1 1 1e-5'

  closures = simple_closures
[]

[FluidProperties]
  [eos]
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
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 50

    A   = 1.0000000000e-04
    D_h  = 1.1283791671e-02

    f = 0.0

    fp = eos
  []

  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe:in'
    p0 = 1e6
    T0 = 453.1
    reversible = false
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 0.5e6
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
  dt = 1.e-2
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-6
  nl_max_its = 30

  l_tol = 1e-3
  l_max_its = 100

  start_time = 0.0
  end_time = 0.6
[]

[Outputs]
  file_base = 'phy.p0T0_3eqn'
  [out]
    type = Exodus
  []
  velocity_as_vector = false
[]
