# Junction between 3 pipes, 1 of which goes to a dead-end. In the steady-state,
# no flow should go into the dead-end pipe.

[GlobalParams]
  gravity_vector = '0 0 0'

  scaling_factor_1phase = '1 1 1e-5'

  initial_T = 250
  initial_p = 1e5
  initial_vel_x = 1
  initial_vel_y = 0
  initial_vel_z = 0

  closures = simple_closures
[]

[AuxVariables]
  [p0]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [p0_kernel]
    type = StagnationPressureAux
    variable = p0
    fp = eos
    e = e
    v = v
    vel = vel
  []
[]

[FluidProperties]
  [eos]
    type = StiffenedGasFluidProperties
    gamma = 1.4
    cv = 725
    q = 0
    q_prime = 0
    p_inf = 0
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Functions]
  [T0]
    type = ParsedFunction
    expression = 'if (x < 1, 300 + 50 * sin(2*pi*x + 1.5*pi), 250)'
  []
[]

[Components]
  [inlet]
    type = InletDensityVelocity1Phase
    input = 'inlet_pipe:in'
    rho = 1.37931034483
    vel = 1
  []

  [inlet_pipe]
    type = FlowChannel1Phase
    fp = eos

    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    A = 1

    f = 0

    initial_T = T0
    initial_p = 1e5
    initial_vel = 1

    n_elems = 20
  []

  [junction1]
    type = VolumeJunction1Phase
    connections = 'inlet_pipe:out deadend_pipe:in outlet_pipe:in'
    position = '1 0 0'
    volume = 1e-8
  []

  [outlet_pipe]
    type = FlowChannel1Phase
    fp = eos

    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    A = 1

    f = 0

    initial_T = 250
    initial_p = 1e5
    initial_vel = 1

    n_elems = 20
  []

  [outlet]
    type = Outlet1Phase
    input = 'outlet_pipe:out'
    p = 1e5
  []

  [deadend_pipe]
    type = FlowChannel1Phase
    fp = eos

    position = '1 0 0'
    orientation = '0 1 0'
    length = 1
    A = 1

    f = 0

    initial_T = 250
    initial_p = 1e5
    initial_vel = 0

    n_elems = 20
  []

  [deadend]
    type = SolidWall1Phase
    input = 'deadend_pipe:out'
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
  nl_max_its = 10

  l_tol = 1e-6
  l_max_its = 10

  start_time = 0
  end_time = 5
  dt = 0.1

  abort_on_solve_fail = true
[]

[Postprocessors]
  # These post-processors are used for testing that the stagnation pressure in
  # the dead-end pipe is equal to the inlet stagnation pressure.
  [p0_inlet]
    type = SideAverageValue
    variable = p0
    boundary = inlet_pipe:in
  []
  [p0_deadend]
    type = SideAverageValue
    variable = p0
    boundary = deadend_pipe:out
  []
  [test_rel_err]
    type = RelativeDifferencePostprocessor
    value1 = p0_deadend
    value2 = p0_inlet
  []
[]

[Outputs]
  [out]
    type = CSV
    show = test_rel_err
    sync_only = true
    sync_times = '1 2 3 4 5'
  []
  velocity_as_vector = false
[]
