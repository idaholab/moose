[GlobalParams]
  initial_p = 1.5e5
  initial_vel = -2
  initial_T = 300

  scaling_factor_1phase = '1 1 1'

  closures = simple
[]

[FluidProperties]
  [./eos]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 28.67055e-3
  [../]
[]

[Components]
  [./pipe1]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '0 0 0'
    orientation = '0 0 1'

    A = 3.14e-4
    D_h = 0.02
    length = 1
    n_elems = 1
    f = 0.01
  [../]

  [./pipe4]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '0 0 2'
    orientation = '0 0 1'

    A = 3.14e-4
    D_h = 0.02
    length = 1
    n_elems = 1
    f = 0.01
  [../]

  [./junction1]
    type = VolumeJunctionOld
    # geometry
    center = '0 0 1.5'

    connections = 'pipe1:out pipe4:in'
    K = '2 3'

    A_ref = 3.14e-2
    volume = 3.14e-2
    initial_T = 628.15
  [../]
[]

[Preconditioning]
  [./SMP_Newton]
    type = SMP
    full = true
    petsc_options_iname = '-snes_type -snes_test_err'
    petsc_options_value = 'test       1e-13'
  [../]
[]


[Executioner]
  type = Transient

  start_time = 0
  dt = 1
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-7
  nl_max_its = 1

  l_tol = 1e-6
  l_max_its = 100

  [./Quadrature]
    type = TRAP
    order = FIRST
  [../]
[]
