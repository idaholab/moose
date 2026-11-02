[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T = 444.447
  initial_p = 7e6
  initial_vel = 0

  closures = simple_closures
[]

[FluidProperties]
  [fp]
    type = Water97FluidProperties
    T_initial_guess = 444.447
    p_initial_guess = 7e6
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
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

  dt = 0.1
  start_time = 0.0
  num_steps = 1

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 0
  nl_abs_tol = 1e-6
  nl_max_its = 20

  l_tol = 1e-3
  l_max_its = 100

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]
