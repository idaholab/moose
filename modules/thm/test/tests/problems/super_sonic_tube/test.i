[GlobalParams]
  gravity_vector = '0 0 0'

  scaling_factor_1phase = '1 1e-2 1e-4'

  initial_p = 101325
  initial_T = 300
  initial_vel = 522.676

  closures = simple_closures

  spatial_discretization = cg
[]

[FluidProperties]
  [ig]
    type = IdealGasFluidProperties
    gamma = 1.41
    molar_mass = 0.028966206103678928
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
    fp = ig
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    A = 1.
    D_h = 1.12837916709551
    f = 0.0

    length = 1
    n_elems = 100
  []

  [inlet]
    type = SupersonicInlet
    input = 'pipe:in'
    p = 101325
    T = 300.0
    vel = 522.676
  []

  [outlet]
    type = FreeBoundary1Phase
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
  scheme = 'bdf2'

  start_time = 0
  dt = 1e-5
  num_steps = 10
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 30

  l_tol = 1e-3
  l_max_its = 100

  [Quadrature]
    type = TRAP
    order = FIRST
  []
[]


[Outputs]
  [out]
    type = Exodus
  []
[]
