# Tests that a flow channel can run with Steady executioner and be set up using Physics
#
# Note that this solve may fail to converge based on initial guess. For example,
# having a guess with velocity set to zero will fail to converge.

[FluidProperties]
  [fp1]
    type = IdealGasFluidProperties
    gamma = 1.4
  []
  [fp2]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Physics]
  [ThermalHydraulics]
    [CompressibleEuler]
      [all]
        scaling_factor_1phase = '1 1 1e-5'
        output_vector_velocity = true
      []
    []
  []
[]

[Components]
  [inlet1]
    type = InletMassFlowRateTemperature
    input = 'pipe1:in'
    m_dot = 2
    T = 500
  []

  [pipe1]
    type = FlowChannel
    position = '0 0 0'
    orientation = '1 0 0'
    gravity_vector = '0 0 0'
    length = 1.0
    n_elems = 50
    A = 1.0

    initial_T = 300
    initial_p = 1e5
    initial_vel = 1

    physics = 'all'
    f = 10.0
    closures = simple_closures
    fp = fp1
  []

  [outlet1]
    type = Outlet
    input = 'pipe1:out'
    p = 2e5
  []

  [inlet2]
    type = InletMassFlowRateTemperature
    input = 'pipe2:in'
    m_dot = 2
    T = 500
  []

  [pipe2]
    type = FlowChannel
    position = '0 0 0'
    orientation = '1 0 0'
    gravity_vector = '0 0 0'
    length = 1.0
    n_elems = 50
    A = 1.0

    # easier initial guess
    initial_T = 480
    initial_p = 1.8e5
    initial_vel = 0.1

    physics = 'all'
    f = 10.0
    closures = simple_closures
    fp = fp2
  []

  [outlet2]
    type = Outlet
    input = 'pipe2:out'
    p = 2e5
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady

  solve_type = NEWTON
  nl_rel_tol = 1e-7
  nl_abs_tol = 1e-7
  nl_max_its = 100

  l_tol = 1e-3
  l_max_its = 10

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Outputs]
  exodus = true
[]
