[GlobalParams]
  initial_vel = 0
  initial_vel_x = 0
  initial_vel_y = 0
  initial_vel_z = 0
  initial_p = 1e5
  initial_T = 300

  f = 0.1
  closures = simple_closures
  fp = fp
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
    type = InletMassFlowRateTemperature1Phase
    input = 'fch1:in'
    m_dot = 1
    T = 300
  []

  [fch1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 1 1'
    length = 1.73205
    n_elems = 5
    A = 1
  []

  [junction]
    type = VolumeJunction1Phase
    position = '1 1 1'
    connections = 'fch1:out fch2:out'
    volume = 0.1
  []

  [fch2]
    type = FlowChannel1Phase
    position = '2 2 2'
    orientation = '-1 -1 -1'
    length = 1.73205
    n_elems = 5
    A = 2
  []

  [outlet]
    type = Outlet1Phase
    input = 'fch2:in'
    p = 1e5
  []
[]

[Executioner]
  type = Transient
  dt = 0.5
  num_steps = 50

  solve_type = NEWTON
  line_search = basic
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_abs_tol = 1e-6
  l_tol = 1e-03

  automatic_scaling = true
[]

[Outputs]
  print_linear_converged_reason = false
  print_nonlinear_converged_reason = false
  print_linear_residuals = false

  [out]
    type = Exodus
    sync_only = false
    sync_times = '0 5 10 15 20 25'
    show = 'vel_x vel_y vel_z'
  []
[]
