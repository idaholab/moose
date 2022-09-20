# Junction between 2 pipes where the second has half the area of the first.
# The momentum density of the second should be twice that of the first.

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T = 300
  initial_p = 1e5
  initial_vel_x = 50
  initial_vel_y = 0
  initial_vel_z = 0

  f = 0

  fp = eos

  scaling_factor_1phase = '1 1e-2 1e-5'

  closures = simple_closures
[]

[FluidProperties]
  [eos]
    type = StiffenedGasFluidProperties
    gamma = 1.4
    cv = 725
    p_inf = 0
    q = 0
    q_prime = 0
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
    T = 250
  []

  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    A = 1
    n_elems = 20
    initial_vel = 20
  []

  [junction]
    type = JunctionParallelChannels1Phase
    connections = 'pipe1:out pipe2:in'
    scaling_factor_rhouV = 1e-4
    scaling_factor_rhoEV = 1e-5
    position = '1 0 0'
    volume = 1e-8
  []

  [pipe2]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    A = 0.5
    n_elems = 20
    initial_vel = 15
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe2:out'
    p = 1e5
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

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 0
  nl_abs_tol = 1e-6
  nl_max_its = 15

  l_tol = 1e-10
  l_max_its = 10

  start_time = 0
  end_time = 3
  dt = 0.1

  abort_on_solve_fail = true
[]

[Postprocessors]
  # These post-processors are used to test that the outlet side of the junction,
  # which has half the area of the inlet side, has twice the momentum density
  # that the inlet side does.
  [rhouA_pipe1]
    type = SideAverageValue
    variable = rhouA
    boundary = pipe1:out
  []
  [rhouA_pipe2]
    type = SideAverageValue
    variable = rhouA
    boundary = pipe2:out
  []
  [test_rel_err]
    type = RelativeDifferencePostprocessor
    value1 = rhouA_pipe1
    value2 = rhouA_pipe2
  []
[]

[Outputs]
  [out]
    type = CSV
    show = test_rel_err
    execute_on = 'final'
  []
[]
[Debug]
  show_var_residual_norms = true
[]
