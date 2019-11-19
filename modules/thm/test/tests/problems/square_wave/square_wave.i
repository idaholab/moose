# Square wave problem

[GlobalParams]
  gravity_vector = '0 0 0'

  spatial_discretization = RDG
  rdg_slope_reconstruction = minmod

  closures = simple
[]

[Functions]
  [./T_ic_fn]
    type = PiecewiseConstant
    axis = x
    direction = right
    x = '0.1 0.6 1.0'
    y = '2.8 1.4 2.8'
  [../]
[]

[FluidProperties]
  [./fp]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 11.64024372
  [../]
[]

[Components]
  [./pipe]
    type = FlowChannel1Phase

    fp = fp

    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 400
    A = 1.0

    # IC
    initial_T = T_ic_fn
    initial_p = 1
    initial_vel = 1

    f = 0
  [../]

  [./left_boundary]
    type = FreeBoundary
    input = 'pipe:in'
  [../]

  [./right_boundary]
    type = FreeBoundary
    input = 'pipe:out'
  [../]
[]

[Executioner]
  type = Transient
  scheme = explicit-midpoint
  solve_type = LINEAR

  l_tol = 1e-4

  nl_rel_tol = 1e-20
  nl_abs_tol = 1e-8
  nl_max_its = 60

  # run to t = 0.3
  start_time = 0.0
  dt = 2e-4
  num_steps = 1500
  abort_on_solve_fail = true
[]

[Outputs]
  file_base = 'square_wave'
  execute_on = 'initial timestep_end'
  [./out]
    type = Exodus
    show = 'p T vel'
  [../]
[]
