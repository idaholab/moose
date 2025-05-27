[GlobalParams]
  initial_T = 393.15
  initial_vel = 0
  initial_p = 17e+06
  f = 0
  fp = fp
  closures = simple_closures
  A = 1
  gravity_vector = '0 0 0'
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
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
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 2
    gravity_vector = '0 0 0'
  []

  [pump]
    type = Pump1Phase
    connections = 'pipe1:out pipe2:in'
    position = '1.02 0 0'
    head = 95
    A_ref = 1
    volume = 1
    initial_vel_x = 0
    initial_vel_y = 0
    initial_vel_z = 0
  []

  [pipe2]
    type = FlowChannel1Phase
    position = '1.04 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 2
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
  dt = 1e-2
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  nl_max_its = 15
  l_tol = 1e-4
  l_max_its = 10

  petsc_options_iname = '-snes_test_err'
  petsc_options_value = '1e-9'

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]
