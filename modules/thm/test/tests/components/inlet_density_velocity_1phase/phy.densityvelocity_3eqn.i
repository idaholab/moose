[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T = 510
  initial_p = 7e6
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

[Components]
  [pipe]
    type = FlowChannel1Phase
    fp = fp
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    A   = 3.1415926536e-06
    D_h  = 2.0000000000e-03
    f = 0.1
    length = 1
    n_elems = 10
  []

  [inlet]
    type = InletDensityVelocity1Phase
    input = 'pipe:in'
    rho = 805
    vel = 1
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

  dt = 1e-1
  start_time = 0.0
  num_steps = 50
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-7
  nl_max_its = 5

  l_tol = 1e-3
  l_max_its = 100
[]

[Outputs]
  exodus = true
  execute_on = 'final'
  velocity_as_vector = false
  show = 'rho vel'
[]
