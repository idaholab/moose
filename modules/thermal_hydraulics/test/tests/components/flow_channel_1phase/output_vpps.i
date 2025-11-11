[GlobalParams]
  gravity_vector = '0 0 0'
  scaling_factor_1phase = '1 1 1e-5'
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe:in'
    p0 = 1e5
    T0 = 300
  []
  [pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 5
    A = 1.0
    initial_T = 300
    initial_p = 1e5
    initial_vel = 0
    fp = fp
    closures = simple_closures
    f = 0
    vpp_vars = 'p T'
    create_flux_vpp = true
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 0.9e5
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
  scheme = bdf2

  start_time = 0
  dt = 1
  num_steps = 1

  solve_type = NEWTON
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 10
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
