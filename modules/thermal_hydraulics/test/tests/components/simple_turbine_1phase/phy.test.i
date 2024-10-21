[GlobalParams]
  initial_p = 1e6
  initial_T = 517
  initial_vel = 1.0
  initial_vel_x = 1
  initial_vel_y = 0
  initial_vel_z = 0

  fp = fp

  closures = simple_closures
  f = 0

  gravity_vector = '0 0 0'
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 0.01
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
    input = 'pipe1:in'
    m_dot = 10
    T = 517
  []

  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10
    A = 1
  []

  [turbine]
    type = SimpleTurbine1Phase
    connections = 'pipe1:out pipe2:in'
    position = '1 0 0'
    volume = 1
    on = true
    power = 1000
    use_scalar_variables = false
  []

  [pipe2]
    type = FlowChannel1Phase
    position = '1. 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10
    A = 1
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe2:out'
    p = 1e6
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
  num_steps = 10

  abort_on_solve_fail = true

  solve_type = 'newton'
  line_search = 'basic'
  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu'

  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-5

  nl_max_its = 5
  l_tol = 1e-4
[]

[Outputs]
  exodus = true
  show = 'p T vel'
  velocity_as_vector = false
  time_step_interval = 5
[]
