[GlobalParams]
  gravity_vector = '0 0 0'

  closures = simple_closures
  fp = fp

  f = 0.0

  initial_T = 300
  initial_p = 1e5
  initial_vel = 0
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 0.02897
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [in]
    type = SupersonicInlet
    input = 'pipe:in'
    vel = 500
    T = 300
    p = 1e5
  []

  [pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 0.5
    n_elems = 2
    A = 0.1
  []

  [out]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 1e5
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2

  solve_type = NEWTON
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 20

  l_tol = 1e-4

  start_time = 0.0
  end_time = 1.0
  dt = 0.01
  abort_on_solve_fail = true
[]
