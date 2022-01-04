[GlobalParams]
  initial_T = 300
  initial_p = 1e5
  initial_vel = 0

  initial_vel_x = 0
  initial_vel_y = 0
  initial_vel_z = 0

  scaling_factor_1phase = '1 1 1'
  scaling_factor_rhoV  = 1
  scaling_factor_rhouV = 1
  scaling_factor_rhovV = 1
  scaling_factor_rhowV = 1
  scaling_factor_rhoEV = 1

  closures = simple_closures
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
  [pipe1a]
    type = FlowChannel1Phase
    fp = fp
    position = '0 0 0'
    orientation = '1 0 0'
    A = 0.785398163e-4    #1.0 cm (0.01 m) in diameter, A = 1/4 * PI * d^2
    D_h = 0.01
    f = 0.01
    length = 0.5
    n_elems = 2
  []

  [pipe1b]
    type = FlowChannel1Phase
    fp = fp
    position = '0.5 0 0'
    orientation = '1 0 0'
    A = 0.785398163e-4    #1.0 cm (0.01 m) in diameter, A = 1/4 * PI * d^2
    D_h = 0.01
    f = 0.01
    length = 0.5
    n_elems = 2
  []

  [pipe2]
    type = FlowChannel1Phase
    fp = fp
    position = '1 0 0'
    orientation = '0 1 0'
    A = 0.785398163e-4    #1.0 cm (0.01 m) in diameter, A = 1/4 * PI * d^2
    D_h = 0.01
    f = 0.01
    length = 1
    n_elems = 3
  []

  [pipe3]
    type = FlowChannel1Phase
    fp = fp
    position = '1 1 0'
    orientation = '-1 0 0'
    A = 0.785398163e-4    #1.0 cm (0.01 m) in diameter, A = 1/4 * PI * d^2
    D_h = 0.01
    f = 0.01
    length = 1
    n_elems = 3
  []

  [pipe4]
    type = FlowChannel1Phase
    fp = fp
    position = '0 1 0'
    orientation = '0 -1 0'
    A = 0.785398163e-4    #1.0 cm (0.01 m) in diameter, A = 1/4 * PI * d^2
    D_h = 0.01
    f = 0.01
    length = 1
    n_elems = 3
  []

  [pipe5]
    type = FlowChannel1Phase
    fp = fp
    position = '1 1 0'
    orientation = '0 1 0'
    A = 0.785398163e-4    #1.0 cm (0.01 m) in diameter, A = 1/4 * PI * d^2
    D_h = 0.01
    f = 0.01
    length = 0.5
    n_elems = 3
  []

  [pump]
    type = Pump1Phase
    connections = 'pipe1a:out pipe1b:in'
    head = 1.0
    position = '0.5 0 0'
    volume = 0.785398163e-3
    A_ref = 0.785398163e-4
  []

  [junction1]
    type = VolumeJunction1Phase
    connections = 'pipe1b:out pipe2:in'
    volume = 0.785398163e-3
    position = '1 0 0'
  []

  [junction2]
    type = VolumeJunction1Phase
    connections = 'pipe2:out pipe3:in pipe5:in'
    volume = 0.785398163e-3
    position = '1 1 0'
  []

  [junction3]
    type = VolumeJunction1Phase
    connections = 'pipe3:out pipe4:in'
    volume = 0.785398163e-3
    position = '0 1 0'
  []

  [junction4]
    type = VolumeJunction1Phase
    connections = 'pipe4:out pipe1a:in'
    volume = 0.785398163e-3
    position = '0 0 0'
  []

  [outlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe5:out'
    p0 = 1.e5
    T0 = 300
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
  num_steps = 10
  dt = 1
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'

  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-7
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 100

  [Quadrature]
    type = gauss
    order = second
  []
[]

[Outputs]
  [out]
    type = Exodus
    show = 'rhouA p'
    execute_on = 'initial final'
  []
[]
