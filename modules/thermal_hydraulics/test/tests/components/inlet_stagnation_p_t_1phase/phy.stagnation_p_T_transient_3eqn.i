[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 101325
  initial_T = 300
  initial_vel = 0

  scaling_factor_1phase = '1 1 1e-4'

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
    fp = eos
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    A = 1.
    f = 0.0

    length = 1
    n_elems = 10
  []

  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe:in'
    p0 = 102041.128
    T0 = 300.615
    reversible = false
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 101325
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

  start_time = 0
  dt = 1e-4
  num_steps = 10
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-7
  nl_max_its = 30

  l_tol = 1e-3
  l_max_its = 100
[]


[Outputs]
  [out]
    type = Exodus
  []
  velocity_as_vector = false
[]
