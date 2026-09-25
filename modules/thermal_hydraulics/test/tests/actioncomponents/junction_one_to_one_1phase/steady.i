# Tests that two flow channel ActionComponents connected by a JunctionOneToOne1Phase
# ActionComponent, with inlet/outlet boundary condition ActionComponents, can run with the Steady
# executioner. The two channels share bare-named flow variables (rhoA, rhouA, rhoEA, A, ...) across
# their blocks, coordinated by THMVariableCoordinator.

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
  []
[]

[ActionComponents]
  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe1:in'
    m_dot = 2
    T = 500
  []

  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 25

    A = 1.0
    fp = fp

    initial_T = 300
    initial_p = 1e5
    initial_vel = 1

    scaling_factor_1phase = '1 1 1e-5'
  []

  [junction]
    type = JunctionOneToOne1Phase
    connections = 'pipe1:out pipe2:in'
  []

  [pipe2]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 25

    A = 1.0
    fp = fp

    initial_T = 300
    initial_p = 1e5
    initial_vel = 1

    scaling_factor_1phase = '1 1 1e-5'
  []

  [outlet]
    type = Outlet1Phase
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
  nl_max_its = 15

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
