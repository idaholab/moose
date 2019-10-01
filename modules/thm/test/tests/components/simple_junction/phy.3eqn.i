[GlobalParams]
  initial_T = 497
  initial_p = 6.0e6
  initial_vel = 0.

  scaling_factor_1phase = '1e1 1e-2 1e-5'

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
    A = 8.78882e-5  #PWR, A = pitch^2 - PI * D_fuel * D_fuel / 4, pitch = 12.6 mm, D_fuel = 9.5 mm
    D_h = 0.01179
    length = 0.5
    n_elems = 50

    f = 0.1
  [../]

  [./ht_pipe_in]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe_in
    Hw = 5.33e4
    P_hf = 0.029832559676
    T_wall = 497
  [../]

  [./br1]
    type = SimpleJunction
    connections = 'pipe_in:out CCH1:in'
  [../]

  # Pipes
  [./CCH1]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '0.5 0 0'
    orientation = '1 0 0'
    A = 8.78882e-5  #PWR, A = pitch^2 - PI * D_fuel * D_fuel / 4, pitch = 12.6 mm, D_fuel = 9.5 mm
    D_h = 0.01179
    length = 3.8
    n_elems = 380

    f = 0.1
  [../]

  [./ht_CCH1]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = CCH1
    Hw = 5.33e4
    P_hf = 0.029832559676
    T_wall = 497
  [../]

  [./br2]
    type = SimpleJunction
    connections = 'CCH1:out pipe_out:in'
  [../]

  [./pipe_out]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '4.3 0 0'
    orientation = '1 0 0'
    A = 8.78882e-5  #PWR, A = pitch^2 - PI * D_fuel * D_fuel / 4, pitch = 12.6 mm, D_fuel = 9.5 mm
    D_h = 0.01179
    length = 0.5
    n_elems = 50

    f = 0.1
  [../]

  [./ht_pipe_out]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe_out
    Hw = 5.33e4
    P_hf = 0.029832559676
    T_wall = 497
  [../]

  [./inlet]
    type = InletStagnationEnthalpyMomentum1Phase
    input = 'pipe_in:in'

    rhou = 0.16484524622
    H =  955731.23683
  [../]

  [./outlet]
    type = Outlet1Phase
    input = 'pipe_out:out'

    p = 6.0e6
    legacy = true
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
  dt = 1e-5
  num_steps = 20
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-5
  nl_max_its = 5

  l_tol = 1e-3
  l_max_its = 30

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
