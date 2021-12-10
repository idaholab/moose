[GlobalParams]
  initial_T = 300
  initial_p = 1e5
  initial_vel = 0
  initial_vel_x = 0
  initial_vel_y = 0
  initial_vel_z = 0
  gravity_vector = '0 0 0'
  scaling_factor_1phase = '1.e0 1.e-4 1.e-6'

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
    A = 1.
    D_h = 1.12837916709551
    f = 0
    length = 1
    n_elems = 10
  []

  [pipe2]
    type = FlowChannel1Phase
    fp = eos
    position = '0 0 0'
    orientation = '0 1 0'
    A = 1.
    D_h = 1.12837916709551
    f = 0
    length = 1
    n_elems = 10
  []

  [pipe3]
    type = FlowChannel1Phase
    fp = eos
    position = '0 0 0'
    orientation = '0 0 1'
    A = 1.
    D_h = 1.12837916709551
    f = 0
    length = 1
    n_elems = 10
  []

  [junction]
    type = VolumeJunction1Phase
    connections = 'pipe1:in pipe2:in pipe3:in'
    position = '0 0 0'
    volume = 1e-5
  []

  [in1]
    type = SolidWall1Phase
    input = 'pipe1:out'
  []
  [in2]
    type = SolidWall1Phase
    input = 'pipe2:out'
  []
  [in3]
    type = SolidWall1Phase
    input = 'pipe3:out'
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
  dt = 1e-5
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-6
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 100
[]

[Outputs]
  exodus = true
  show = 'A'
[]
