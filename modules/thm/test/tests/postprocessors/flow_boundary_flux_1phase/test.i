T_in = 300
p_out = 1e5

[GlobalParams]
  initial_p = ${p_out}
  initial_T = ${T_in}
  initial_vel = 0
  gravity_vector = '0 0 0'
  closures = simple_closures
  n_elems = 50
  f = 0
  scaling_factor_1phase = '1 1e-2 1e-4'
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
  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'channel:in'
    m_dot = 0.1
    T = ${T_in}
  []
  [channel]
    type = FlowChannel1Phase
    fp = fp
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    A = 3
  []
  [outlet]
    type = Outlet1Phase
    p = ${p_out}
    input = 'channel:out'
  []
[]

[Postprocessors]
  [m_dot_in]
    type = ADFlowBoundaryFlux1Phase
    boundary = 'inlet'
    equation = mass
  []
  [m_dot_out]
    type = ADFlowBoundaryFlux1Phase
    boundary = 'outlet'
    equation = mass
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  start_time = 0
  num_steps = 10
  dt = 0.1

  solve_type = NEWTON
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 25
  l_tol = 1e-3
  l_max_its = 5
[]

[Outputs]
  [out]
    type = CSV
    show = 'm_dot_in m_dot_out'
    execute_on = 'final'
  []
[]
