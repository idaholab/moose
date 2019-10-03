[GlobalParams]
  initial_T = 497
  initial_p = 6.0e6
  initial_vel = 2.

  scaling_factor_1phase = '1 1 1'

  closures = simple
[]

[FluidProperties]
  [./eos]
    type = StiffenedGasFluidProperties
    gamma = 1.43
    cv = 1040.0
    q = 2.03e6
    p_inf = 0.0
    q_prime = -2.3e4
  [../]
[]

[Components]
  [./pipe_in]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    A = 8.78882e-5
    D_h = 0.01179
    length = 0.5
    n_elems = 1

    f = 0.1
  [../]

  [./ht_pipe_in]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe_in
    T_wall = 497
    Hw = 5.33e4
    P_hf = 0.029832559676
  [../]

  [./br1]
    type = SimpleJunction
    connections = 'pipe_in:out pipe_out:in'
  [../]

  [./pipe_out]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '4.3 0 0'
    orientation = '1 0 0'
    A = 8.78882e-5
    D_h = 0.01179
    length = 0.5
    n_elems = 1

    f = 0.1
  [../]

  [./ht_pipe_out]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe_out
    T_wall = 497
    Hw = 5.33e4
    P_hf = 0.029832559676
  [../]
[]

[Preconditioning]
  [./SMP_Newton]
    type = SMP
    full = true

    petsc_options_iname = '-snes_type -snes_test_err'
    petsc_options_value = 'test       1e-12'
  [../]
[]

[Executioner]
  type = Transient

  start_time = 0
  dt = 1
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-5
  nl_max_its = 1

  l_tol = 1e-3
  l_max_its = 30

  [./Quadrature]
    type = TRAP
    order = FIRST
  [../]
[]
