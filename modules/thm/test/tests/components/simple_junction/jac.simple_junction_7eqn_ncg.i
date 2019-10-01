[GlobalParams]
  initial_T_liquid = 483
  initial_T_vapor  = 550
  initial_p_liquid = 4e6
  initial_p_vapor  = 4e6
  initial_vel_liquid = 2
  initial_vel_vapor = 2
  initial_alpha_vapor = 0.5
  initial_x_ncgs = '0.1'

  gravity_vector = '0 0 0'

  scaling_factor_2phase = '1 1 1 1 1 1 1'

  pressure_relaxation = false
  velocity_relaxation = false
  interface_transfer = false
  wall_mass_transfer = false

  specific_interfacial_area_max_value = 10

  closures = simple
[]

[FluidProperties]
  [./eos]
    type = StiffenedGas7EqnFluidProperties
  [../]
[]

[Components]
  [./pipe1]
    type = FlowChannel2Phase
    fp = eos
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    A = 1e-4
    D_h = 1.12837916709551
    f = 0.01
    f_interface = 0

    length = 1
    n_elems = 1
  [../]

  [./br1]
    type = SimpleJunction
    connections = 'pipe1:out pipe2:in'
  [../]

  [./pipe2]
    type = FlowChannel2Phase
    fp = eos
    # geometry
    position = '1 0 0'
    orientation = '1 0 0'
    A = 1e-4
    D_h = 1.12837916709551
    f = 0.01
    f_interface = 0

    length = 1
    n_elems = 1
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
  nl_rel_tol = 1e-13
  nl_abs_tol = 1e-9
  nl_max_its = 1

  l_tol = 1e-3
  l_max_its = 100

  [./Quadrature]
    type = TRAP
    order = FIRST
  [../]
[]
