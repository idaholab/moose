# Tests that two flow channel ActionComponents connected by a VolumeJunction1Phase ActionComponent,
# with inlet/outlet boundary condition ActionComponents, can run transient. The junction has its own
# mesh (a NodeElem), its own solver/auxiliary variables, and shares bare variable names (rhoV, p, ...)
# with any other volume junction in the simulation via THMVariableCoordinator, exactly as the two
# flow channels already share rhoA, rhouA, rhoEA, A, ... with each other.

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
    type = VolumeJunction1Phase
    connections = 'pipe1:out pipe2:in'
    volume = 1e-3
    position = '1 0 0'

    initial_p = 1e5
    initial_T = 300
    initial_vel_x = 1
    initial_vel_y = 0
    initial_vel_z = 0
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
  type = Transient

  start_time = 0
  dt = 0.01
  num_steps = 10

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

[Postprocessors]
  [junction_p]
    type = ElementAverageValue
    variable = p
    block = 'junction'
    execute_on = 'initial timestep_end'
  []
  [junction_T]
    type = ElementAverageValue
    variable = T
    block = 'junction'
    execute_on = 'initial timestep_end'
  []
  [junction_vel]
    type = ElementAverageValue
    variable = vel
    block = 'junction'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'initial timestep_end'
  []
[]
