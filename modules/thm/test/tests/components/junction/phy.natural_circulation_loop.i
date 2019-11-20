# Tests a natural circulation loop.

[GlobalParams]
  gravity_vector = '-9.8 0 0'

  scaling_factor_1phase = '1 1 1e-5'

  initial_p = 1e5
  initial_T = 300
  initial_vel = 0

  # pipe global parameters
  A = 1
  P_hf = 1
  f = 0

  closures = simple

  spatial_discretization = cg
[]

[FluidProperties]
  [./eos]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
  []
[]

[Components]
  # "pressurizer"
  [./pressurizer]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe8:out'
    p0 = 1e5
    T0 = 300
  [../]

  # pipes
  [./pipe1]
    type = FlowChannel1Phase
    fp = eos
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 20
  [../]
  [./ht_pipe1]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe1
    T_wall = 350
    Hw = 1e5
  [../]
  [./pipe2]
    type = FlowChannel1Phase
    fp = eos
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 20
  [../]
  [./pipe3]
    type = FlowChannel1Phase
    fp = eos
    position = '2 0 0'
    orientation = '0 1 0'
    length = 1
    n_elems = 20
  [../]
  [./pipe4]
    type = FlowChannel1Phase
    fp = eos
    position = '2 1 0'
    orientation = '0 1 0'
    length = 1
    n_elems = 20
  [../]
  [./pipe5]
    type = FlowChannel1Phase
    fp = eos
    position = '2 2 0'
    orientation = '-1 0 0'
    length = 1
    n_elems = 20
  [../]
  [./ht_pipe5]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe5
    T_wall = 250
    Hw = 1e5
  [../]
  [./pipe6]
    type = FlowChannel1Phase
    fp = eos
    position = '1 2 0'
    orientation = '-1 0 0'
    length = 1
    n_elems = 20
  [../]
  [./pipe7]
    type = FlowChannel1Phase
    fp = eos
    position = '0 2 0'
    orientation = '0 -1 0'
    length = 2
    n_elems = 20
  [../]
  [./pipe8]
    type = FlowChannel1Phase
    fp = eos
    position = '2 1 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 20
  [../]

  # junctions
  [./junction1]
    type = Junction
    connections = 'pipe1:out pipe2:in'
  [../]
  [./junction2]
    type = Junction
    connections = 'pipe2:out pipe3:in'
  [../]
  [./junction3]
    type = Junction
    connections = 'pipe3:out pipe4:in pipe8:in'
  [../]
  [./junction4]
    type = Junction
    connections = 'pipe4:out pipe5:in'
  [../]
  [./junction5]
    type = Junction
    connections = 'pipe5:out pipe6:in'
  [../]
  [./junction6]
    type = Junction
    connections = 'pipe6:out pipe7:in'
  [../]
  [./junction7]
    type = Junction
    connections = 'pipe7:out pipe1:in'
  [../]
[]

[Preconditioning]
  [./pc]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
    petsc_options_value = 'lu       mumps'
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 0
  nl_abs_tol = 1e-5
  nl_max_its = 10

  l_tol = 1e-10
  l_max_its = 10

  start_time = 0
  end_time = 5000

  steady_state_detection = true
  steady_state_tolerance = 1e-6

  dt = 1.0
  abort_on_solve_fail = true

  [./Quadrature]
    type = GAUSS
    order = SECOND
  [../]
[]

[Outputs]
  [./out]
    type = Exodus
    show = T
    execute_on = final
  [../]
[]
