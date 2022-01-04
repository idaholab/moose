[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T = 517.252072255516
  initial_vel = 0

  scaling_factor_1phase = '1.e0 1.e0 1.e-2'

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

[Functions]
  [p_fn]
    type = PiecewiseConstant
    axis = x
    x = '0      0.5    1'
    y = '7.5e6  6.5e6  6.5e6'
  []
[]

[Components]
  [pipe1]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 200

    A = 1.907720E-04
    D_h = 1.698566E-02
    f = 0.

    initial_p = p_fn
  []

  # BCs
  [left]
    type = SolidWall1Phase
    input = 'pipe1:in'
  []

  [right]
    type = SolidWall1Phase
    input = 'pipe1:out'
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

  start_time = 0
  dt = 1e-5
  num_steps = 10
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-9
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 100
[]

[Outputs]
  velocity_as_vector = false
  [out]
    type = Exodus
  []
[]
