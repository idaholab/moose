[GlobalParams]
  initial_p = 1e6
  initial_T = 517
  initial_vel = 1.0
  initial_vel_x = 1
  initial_vel_y = 0
  initial_vel_z = 0

  f = 0
  fp = fp
  closures = simple_closures
  gravity_vector = '0 0 0'

  automatic_scaling = true
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 1.43
    cv = 1040.0
    q = 2.03e6
    p_inf = 0.0
    q_prime = -2.3e4
    k = 0.026
    mu = 134.4e-7
    M = 0.01801488
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Functions]
  [W_dot_fn]
    type = PiecewiseLinear
    xy_data = '
      0 0
      1 10'
  []
[]

[Components]
  [inlet]
    type = InletVelocityTemperature1Phase
    input = 'pipe1:in'
    vel = 1
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
    A_ref = 1.0
    K = 0
    on = true
    power = 0
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

[ControlLogic]
  [W_dot_ctrl]
    type = TimeFunctionComponentControl
    component = turbine
    parameter = power
    function = W_dot_fn
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
  dt = 0.1
  num_steps = 10

  solve_type = 'newton'
  line_search = 'basic'
  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-3
  nl_max_its = 5
  l_tol = 1e-4

  abort_on_solve_fail = true
[]

[Postprocessors]
  [W_dot]
    type = ScalarVariable
    variable = turbine:W_dot
  []
[]

[Outputs]
  [csv]
    type = CSV
    show = 'W_dot'
  []
[]
