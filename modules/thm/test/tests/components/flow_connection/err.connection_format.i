[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 28.964e-3
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [left_wall]
    type = SolidWall1Phase
  []

  [pipe]
    type = FlowChannel1Phase
    fp = fp
    closures = simple_closures

    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 5
    A = 1.0

    initial_T = 300
    initial_p = 1e5
    initial_vel = 0

    f = 0
  []

  [right_wall]
    type = SolidWall1Phase
    input = 'pipe:out'
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  nl_rel_tol = 0
  nl_abs_tol = 1e-6
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 10

  start_time = 0.0
  dt = 0.01
  num_steps = 1

  abort_on_solve_fail = true

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]
