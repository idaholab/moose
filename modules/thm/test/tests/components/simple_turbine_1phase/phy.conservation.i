[GlobalParams]
  initial_p = 1e6
  initial_T = 517
  initial_vel = 1.0
  initial_vel_x = 1
  initial_vel_y = 0
  initial_vel_z = 0

  fp = fp

  closures = simple
  f = 0

  gravity_vector = '0 0 0'
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
    power = 1000
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

[Postprocessors]
  [mass_in]
    type = ADFlowBoundaryFlux1Phase
    equation = mass
    boundary = inlet
  []
  [mass_out]
    type = ADFlowBoundaryFlux1Phase
    equation = mass
    boundary = outlet
  []
  [mass_diff]
    type = LinearCombinationPostprocessor
    pp_coefs = '1 -1'
    pp_names = 'mass_in mass_out'
  []

  [energy_in]
    type = ADFlowBoundaryFlux1Phase
    equation = energy
    boundary = inlet
  []
  [energy_out]
    type = ADFlowBoundaryFlux1Phase
    equation = energy
    boundary = outlet
  []
  [W_dot]
    type = ScalarVariable
    variable = turbine:W_dot
  []
  [energy_diff]
    type = LinearCombinationPostprocessor
    pp_coefs = '1 -1 -1'
    pp_names = 'energy_in energy_out W_dot'
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
  num_steps = 30

  abort_on_solve_fail = true

  solve_type = 'newton'
  line_search = 'basic'

  nl_rel_tol = 1e-7
  nl_abs_tol = 2e-5

  nl_max_its = 5
  l_tol = 1e-4
[]

[Outputs]
  [csv]
    type = CSV
    show = 'mass_diff energy_diff'
    execute_on = 'final'
  []
[]
