[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 1e5
  initial_T = 300
  initial_vel = 0

  closures = simple_closures
[]

[FluidProperties]
  [eos]
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
    fp = eos

    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10
    A = 1

    f = 0
  []

  [pipe2]
    type = FlowChannel1Phase
    fp = eos

    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10
    A = 1

    f = 0
  []

  [junction]
    type = VolumeJunction1Phase
    connections = 'pipe1:out pipe2:in'
    volume = 1e-5
    position = '1 0 0'

    initial_vel_x = 0
    initial_vel_y = 0
    initial_vel_z = 0
  []

  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe1:in'
    # (p0, T0) for p = 1e5, T = 300, vel = 1
    p0 = 1.0049827846e+05
    T0 = 300.0000099
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe2:out'
    p = 1e5
  []
[]

[Preconditioning]
  [prec]
    type = SMP
    full = true
    petsc_options = '-pc_factor_shift_nonzero'
    petsc_options_iname = '-mat_fd_coloring_err'
    petsc_options_value = '1.e-10'
  []
[]

[Executioner]
  type = Transient

  start_time = 0
  dt = 1e-4
  num_steps = 5
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  nl_rel_tol = 0
  nl_abs_tol = 1e-5
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 10

  [Adaptivity]
    initial_adaptivity = 0
    refine_fraction = 0.60
    coarsen_fraction = 0.10
    max_h_level = 3
  []

  automatic_scaling = true
[]

[Outputs]
  exodus = true
[]
