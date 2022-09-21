[GlobalParams]
  initial_p = 9.5e4
  initial_T = 310
  initial_vel = 2

  gravity_vector = '9.81 0 0'

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
  [pipe]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    A = 1e-4
    D_h = 1.12837916709551
    f = 0.0

    length = 1
    n_elems = 3
  []

  [inlet]
    type = SolidWall1Phase
    input = 'pipe:in'
  []

  [outlet]
    type = SolidWall1Phase
    input = 'pipe:out'
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

  start_time = 0
  dt = 1
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 30

  l_tol = 1e-3
  l_max_its = 100

  petsc_options_iname = '-snes_type -snes_test_err'
  petsc_options_value = 'test       1e-11'
[]
