# Tests the OldJunction component with 2 pipes going in and 3 pipes going out,
# with no form loss at the junction, only friction along the pipes.

[GlobalParams]
  initial_p = 1.e5
  initial_vel = 0.
  initial_T = 300.

  closures = simple
[]

[FluidProperties]
  [./eos]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816
    q = -1.167e6
    p_inf = 1e9
    q_prime = 0
  [../]
[]

[Components]
  [./pipe1]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    A = 2.
    length = 1
    # Big value of f since A is way too large.
    # To create a more realistic model, we can shrink pipe size and friction factor simultaneously.
    f = 5000.0
    n_elems = 50
  [../]

  [./ht_pipe1]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe1
    T_wall = 300
    Hw = 1e4
  [../]

  [./pipe2]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '0 0.3 0'
    orientation = '1 0 0'
    A = 1.
    length = 1
    f = 5000.0
    n_elems = 50
  [../]

  [./ht_pipe2]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe2
    T_wall = 300
    Hw = 1e4
  [../]

  [./pipe3]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '1.2 0 0'
    orientation = '1 0 0'
    A = 1.
    length = 1
    f = 5000.0
    n_elems = 50
  [../]

  [./ht_pipe3]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe3
    T_wall = 300
    Hw = 1e4
  [../]

  [./pipe4]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '1.2 0.5 0'
    orientation = '1 0 0'
    A = 1.
    length = 1
    f = 5000.0
    n_elems = 50
  [../]

  [./ht_pipe4]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe4
    T_wall = 300
    Hw = 1e4
  [../]

  [./pipe5]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '1.2 1.0 0'
    orientation = '1 0 0'
    A = 1.5
    length = 1
    f = 5000.0
    n_elems = 50
  [../]

  [./ht_pipe5]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe5
    T_wall = 300
    Hw = 1e4
  [../]

  [./junction]
    type = OldJunction
    connections = 'pipe1:out pipe2:out pipe3:in pipe4:in pipe5:in'
    K = '0 0 0 0 0'
    A_ref = 3.0
  [../]

  [./inlet_1]
    type = InletDensityVelocity1Phase
    input = 'pipe1:in'
    rho = 995
    vel = 1
  [../]
  [./inlet_2]
    type = InletDensityVelocity1Phase
    input = 'pipe2:in'
    rho = 995
    vel = 1
  [../]

  [./outlet_3]
    type = Outlet1Phase
    input = 'pipe3:out'
    p = 9.5e4
    legacy = true
  [../]
  [./outlet_4]
    type = Outlet1Phase
    input = 'pipe4:out'
    p = 9.5e4
    legacy = true
  [../]
  [./outlet_5]
    type = Outlet1Phase
    input = 'pipe5:out'
    p = 9.5e4
    legacy = true
  [../]
[]

[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
    petsc_options_value = ' lu       NONZERO               1e-10'
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  dt = 0.001
  num_steps = 10
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-9
  nl_max_its = 10

  l_tol = 1e-3
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
