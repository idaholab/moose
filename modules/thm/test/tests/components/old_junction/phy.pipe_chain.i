# Tests the OldJunction component with 3 pipes in series.

[GlobalParams]
  initial_p = 1.e5
  initial_vel = 0.
  initial_T = 300.

  scaling_factor_1phase = '1e4 1 1e-2'

  closures = simple
[]

[FluidProperties]
  [./eos]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
  [../]
[]


[Components]
  [./pipe1]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    A = 0.785398163e-4    #1.0 cm (0.01 m) in diameter, A = 1/4 * PI * d^2
    D_h = 0.01
    f = 0.01
    length = 1
    n_elems = 20
  [../]

  [./pipe2]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '1.2 0 0'
    orientation = '1 0 0'
    A = 0.785398163e-4    #1.0 cm (0.01 m) in diameter, A = 1/4 * PI * d^2
    D_h = 0.01
    f = 0.01
    length = 1
    n_elems = 20
  [../]

  [./pipe3]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '2.4 0 0'
    orientation = '1 0 0'
    A = 0.785398163e-4    #1.0 cm (0.01 m) in diameter, A = 1/4 * PI * d^2
    D_h = 0.01
    f = 0.01
    length = 1
    n_elems = 20
  [../]

  [./junction12]
    type = OldJunction
    connections = 'pipe1:out pipe2:in'
    K = '0 0'
    A_ref = 1.5e-3
  [../]

  [./junction23]
    type = OldJunction
    connections = 'pipe2:out pipe3:in'
    K = '0 0'
    A_ref = 1.5e-3
  [../]

  [./inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe1:in'
    p0 = 1.1e5
    T0 = 301.0
  [../]
  [./outlet]
    type = Outlet1Phase
    input = 'pipe3:out'
    p = 1.0e5
  [../]
[]


[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true

  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  dt = 5e-4
  num_steps = 20
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-8
  l_max_its = 100

  [./Quadrature]
    type = TRAP
    order = FIRST
  [../]
[]


[Outputs]
  [./out]
    type = Exodus
  [../]
[]
