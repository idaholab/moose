# Junction between 3 pipes, 1 of which goes to a dead-end. In the steady-state,
# no flow should go into the dead-end pipe.

[GlobalParams]
  gravity_vector = '0 0 0'

  scaling_factor_1phase = '1 1 1e-5'

  # Currently, these parameters are required for the initialization of junction DoFs
  initial_T = 250
  initial_p = 1e5

  closures = simple

  spatial_discretization = cg
[]

[AuxVariables]
  [./p0]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[AuxKernels]
  [./p0_kernel]
    type = StagnationPressureAux
    variable = p0
    fp = eos
    e = e
    v = v
    vel = vel
  [../]
[]

[FluidProperties]
  [./eos]
    type = StiffenedGasFluidProperties
    gamma = 1.4
    cv = 725
    q = 0
    q_prime = 0
    p_inf = 0
  [../]
[]

[Functions]
  [./T0]
    type = ParsedFunction
    value = 'if (x < 1, 300 + 50 * sin(2*pi*x + 1.5*pi), 250)'
  [../]
[]

[Components]
  [./inlet]
    type = InletDensityVelocity1Phase
    input = 'inlet_pipe:in'
    rho = 1.37931034483
    vel = 1
  [../]

  [./inlet_pipe]
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
  [../]

  [./junction1]
    type = Junction
    connections = 'inlet_pipe:out deadend_pipe:in outlet_pipe:in'
  [../]

  [./outlet_pipe]
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
  [../]

  [./outlet]
    type = Outlet1Phase
    input = 'outlet_pipe:out'
    p = 1e5
  [../]

  [./deadend_pipe]
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
  [../]

  [./deadend]
    type = SolidWall
    input = 'deadend_pipe:out'
  [../]
[]

[Preconditioning]
  [./pc]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
    petsc_options_value = 'lu       mumps'
  [../]
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

  dt = 0.05
  abort_on_solve_fail = true

  [./Quadrature]
    type = GAUSS
    order = SECOND
  [../]
[]

[Postprocessors]
  # These post-processors are used for testing that the stagnation pressure in
  # the dead-end pipe is equal to the inlet stagnation pressure.
  [./p0_inlet]
    type = PointValue
    variable = p0
    point = '0 0 0'
  [../]
  [./p0_deadend]
    type = PointValue
    variable = p0
    point = '1 1 0'
  [../]
  [./test_rel_err]
    type = RelativeDifferencePostprocessor
    value1 = p0_deadend
    value2 = p0_inlet
  [../]
[]

[Outputs]
  [./out]
    type = CSV
    show = test_rel_err
  [../]
[]
