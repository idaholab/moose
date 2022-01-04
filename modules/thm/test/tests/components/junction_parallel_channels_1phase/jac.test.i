# Pump data used in this test comes from the LOFT Systems Tests, described in NUREG/CR-0247

[GlobalParams]
  initial_p = 1e5
  initial_T = 300
  initial_vel = 1

  closures = simple_closures
  fp = fp
  f = 0

  scaling_factor_1phase = '1e-2 1e-2 1e-5'
  scaling_factor_rhoEV = 1e-5
[]

[FluidProperties]
  [fp]
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
  [in]
    type = InletStagnationPressureTemperature1Phase
    input = fch1:in
    p0 = 1.1e5
    T0 = 300
  []

  [fch1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 1
    A = 1
  []

  [junction]
    type = JunctionParallelChannels1Phase
    connections = 'fch1:out fch2:in'
    position = '1 0 0'
    volume = 0.3
    initial_vel_x = 1
    initial_vel_y = 0
    initial_vel_z = 0
  []

  [fch2]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 1
    A = 1.5
  []

  [out]
    type = Outlet1Phase
    input = fch2:out
    p = 1e5
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
  num_steps = 1
  abort_on_solve_fail = true
  dt = 0.1

  solve_type = 'newton'
  line_search = 'basic'
  petsc_options_iname = '-snes_test_err'
  petsc_options_value = '1e-9'

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  nl_max_its = 15

  l_tol = 1e-4
  l_max_its = 10
[]
