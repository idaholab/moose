[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 6e6
  initial_T = 600
  initial_vel = 0

  scaling_factor_1phase = '1 1 1e-5'

  closures = simple_closures
[]

[FluidProperties]
  [eos]
    type = IdealGasFluidProperties
    gamma = 1.3066
    molar_mass = 2.016e-3
    k = 0.437
    mu = 3e-5
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [pipe1]
    type = FlowChannel1Phase
    fp = eos

    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10

    A = 0.1

    f = 0.
  []

  [inlet]
    type = InletMassFlowRateTemperature1Phase
    m_dot = 0.1
    T = 800
    input = 'pipe1:in'
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe1:out'
    p = 6e6
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

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.01
    growth_factor = 1.4
    optimal_iterations = 6
    iteration_window = 2
  []
  start_time = 0.0
  end_time = 100
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 10
[]

[Postprocessors]
  # hand calcs show that Isp should start at 274.3 at 600 K
  # and rise to 316.7 at 800 K.
  [Isp]
    type = ADSpecificImpulse1Phase
    p_exit = 1e6
    fp = eos
    boundary = outlet
  []

  [Isp_inst]
    type = ADSpecificImpulse1Phase
    p_exit = 1e6
    fp = eos
    cumulative = false
    boundary = outlet
  []

  [outletT]
    type = SideAverageValue
    variable = T
    boundary = pipe1:out
  []
[]

[Outputs]
  [out]
    type = CSV
    show = 'Isp Isp_inst'
    execute_on = 'INITIAL FINAL'
  []
[]
