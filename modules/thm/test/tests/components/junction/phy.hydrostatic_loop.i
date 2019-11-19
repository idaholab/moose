# Tests to see if a hydrostatic pressure distribution develops in a network of pipes.

[GlobalParams]
  gravity_vector = '9.8 0 0'

  initial_T = 300
  initial_p = 1e5
  initial_vel = 0

  # global parameters for pipes
  fp = eos
  length = 1
  n_elems = 20
  A = 1
  f = 0

  scaling_factor_1phase = '1 1 1e-5'

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
  [./inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe1:in'
    p0 = 1e5
    T0 = 300
  [../]

  [./pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
  [../]
  [./pipe2]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '1 0 0'
  [../]
  [./pipe3]
    type = FlowChannel1Phase
    position = '2 0 0'
    orientation = '0 1 0'
  [../]
  [./pipe4]
    type = FlowChannel1Phase
    position = '2 1 0'
    orientation = '-1 0 0'
  [../]
  [./pipe5]
    type = FlowChannel1Phase
    position = '1 1 0'
    orientation = '0 -1 0'
  [../]

  [./junction1]
    type = Junction
    connections = 'pipe1:out pipe5:out pipe2:in'
  [../]
  [./junction2]
    type = Junction
    connections = 'pipe2:out pipe3:in'
  [../]
  [./junction3]
    type = Junction
    connections = 'pipe3:out pipe4:in'
  [../]
  [./junction4]
    type = Junction
    connections = 'pipe4:out pipe5:in'
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
  end_time = 5
  steady_state_detection = true
  steady_state_tolerance = 2e-7

  dt = 0.05
  abort_on_solve_fail = true

  [./Quadrature]
    type = GAUSS
    order = SECOND
  [../]
[]

[Outputs]
  [./out]
    type = Exodus
    execute_on = final
  [../]
[]
