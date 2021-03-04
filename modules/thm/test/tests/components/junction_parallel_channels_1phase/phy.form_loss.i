# This test measures the pressure drop across the junction with K=1.
# The observed value should be ~4.9892 Pa, which is the value computed by the
# volume junction term 'K * (p0 - p)' in steady state.

A = 0.1

[GlobalParams]
  gravity_vector = '0 0 0'

  scaling_factor_1phase = '1 1e-3 1e-5'
  scaling_factor_rhoV  = 1
  scaling_factor_rhouV = 1e-3
  scaling_factor_rhovV = 1
  scaling_factor_rhowV = 1
  scaling_factor_rhoEV = 1e-5

  initial_T = 300
  initial_p = 1e5
  initial_vel = 4

  n_elems = 20
  length = 1

  f = 0

  fp = fp

  closures = simple
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 1.4
    cv = 725
    q = 0
    q_prime = 0
    p_inf = 0
  []
[]

[Components]
  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    A = ${A}
  []

  [pipe2]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '1 0 0'
    A = ${A}
    initial_p = 1e5
  []


  [junction]
    type = JunctionParallelChannels1Phase
    connections = 'pipe1:out pipe2:in'

    position = '1 0 0'
    volume = 0.37

    initial_p = 1e5
    initial_vel_x = 0
    initial_vel_y = 0
    initial_vel_z = 0
  []

  [pipe1_in]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe1:in'
    p0 = 1.0001e5
    T0 = 300
    reversible = false
  []

  [pipe2_out]
    type = Outlet1Phase
    input = 'pipe2:out'
    p = 1e5
  []
[]

[Postprocessors]
  [pJ_in]
    type = SideAverageValue
    variable = p
    boundary = pipe1:out
  []

  [pJ_out]
    type = SideAverageValue
    variable = p
    boundary = pipe2:in
  []

  [dpJ]
    type = DifferencePostprocessor
    value1 = pJ_in
    value2 = pJ_out
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

  start_time = 0
  end_time = 20
  dt = 0.5
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  nl_rel_tol = 0
  nl_abs_tol = 1e-6
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 10
[]


[Outputs]
  csv = true
  execute_on = 'final'
  show = 'dpJ'
[]
