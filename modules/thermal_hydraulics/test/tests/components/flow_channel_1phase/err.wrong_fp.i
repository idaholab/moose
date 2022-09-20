[GlobalParams]
  closures = simple_closures

  initial_p = 1e5
  initial_T = 300
  initial_vel = 0
[]

[FluidProperties]
  [fp_2phase]
    type = StiffenedGasTwoPhaseFluidProperties
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
    position = '0 0 0'
    orientation = '1 0 0'
    A = 1.
    D_h = 1.12837916709551
    f = 0.01
    length = 1
    n_elems = 100
    fp = fp_2phase   # this is wrong
  []

  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe:in'
    p0 = 1e5
    T0 = 300
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 9.5e4
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  dt = 1e-4
  dtmin = 1.e-7

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-8
  l_max_its = 100

  start_time = 0.0
  num_steps = 1
[]
