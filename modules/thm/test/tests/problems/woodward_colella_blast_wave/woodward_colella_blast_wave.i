# Woodward-Colella blast wave problem

[GlobalParams]
  gravity_vector = '0 0 0'

  closures = simple_closures
[]

[Functions]
  [p_ic_fn]
    type = PiecewiseConstant
    axis = x
    direction = right
    x = '0.1  0.9  1.0'
    y = '1000 0.01 100'
  []
  [T_ic_fn]
    type = PiecewiseConstant
    axis = x
    direction = right
    x = '0.1  0.9   1.0'
    y = '1400 0.014 140'
  []
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 11.64024372
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

    fp = fp

    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 500
    A = 1.0

    # IC
    initial_T = T_ic_fn
    initial_p = p_ic_fn
    initial_vel = 0

    f = 0
  []

  [left_wall]
    type = SolidWall1Phase
    input = 'pipe:in'
  []

  [right_wall]
    type = SolidWall1Phase
    input = 'pipe:out'
  []
[]

[Executioner]
  type = Transient
  [TimeIntegrator]
    type = ExplicitSSPRungeKutta
    order = 2
  []
  solve_type = LINEAR

  l_tol = 1e-4

  nl_rel_tol = 1e-20
  nl_abs_tol = 1e-8
  nl_max_its = 60

  # run to t = 0.038
  start_time = 0.0
  dt = 1e-5
  num_steps = 3800
  abort_on_solve_fail = true
[]

[Outputs]
  file_base = 'woodward_colella_blast_wave'
  velocity_as_vector = false
  execute_on = 'initial timestep_end'
  [out]
    type = Exodus
    show = 'p T vel'
  []
[]
