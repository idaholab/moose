[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 101325
  initial_T = 300
  initial_vel = 0

  scaling_factor_1phase = '1.e2 1. 1.e-3'

  closures = simple_closures
[]

[FluidProperties]
  [eos]
    type = IdealGasFluidProperties
    gamma = 1.41
    molar_mass = 28.9662e-3
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
    D_h  = 1.1283791671e-02

    f = 0.0

    length = 1
    n_elems = 100
  []

  [inlet]
    type = InletStagnationEnthalpyMomentum1Phase
    input = 'pipe:in'
    H    = 296748.357480000
    rhou = 41.0009888754850
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 101325
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
  dt = 1.e-2
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-14
  nl_abs_tol = 5e-8
  nl_max_its = 30

  l_tol = 1e-3
  l_max_its = 100

  start_time = 0.0
  end_time = 0.2
[]
