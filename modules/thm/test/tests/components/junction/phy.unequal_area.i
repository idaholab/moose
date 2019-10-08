# Junction between 2 pipes where the second has half the area of the first.
# The momentum density of the second should be twice that of the first.

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T = 250
  initial_p = 1e5
  initial_vel = 1

  f = 0

  fp = eos

  scaling_factor_1phase = '1 1 1e-5'

  closures = simple
[]

[FluidProperties]
  [./eos]
    type = StiffenedGasFluidProperties
    gamma = 1.4
    cv = 725
    p_inf = 0
    q = 0
    q_prime = 0
  [../]
[]

[Components]
  [./inlet]
    type = InletDensityVelocity1Phase
    input = 'pipe1:in'
    rho = 1.37931034483
    vel = 1
  [../]

  [./pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    A = 1
    n_elems = 20
  [../]

  [./junction]
    type = Junction
    connections = 'pipe1:out pipe2:in'
  [../]

  [./pipe2]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    A = 0.5
    n_elems = 20
  [../]

  [./outlet]
    type = Outlet1Phase
    input = 'pipe2:out'
    p = 1e5
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

  l_tol = 1e-10
  l_max_its = 10

  start_time = 0
  end_time = 5
  steady_state_detection = true
  steady_state_tolerance = 2e-7

  dt = 0.05
  abort_on_solve_fail = true

  [./Quadrature]
    type = GAUSS
    order = SECOND
  [../]
[]

[Postprocessors]
  # These post-processors are used to test that the outlet side of the junction,
  # which has half the area of the inlet side, has twice the momentum density
  # that the inlet side does.
  [./rhouA_small]
    type = PointValue
    variable = rhouA
    point = '0.999 0 0'
  [../]
  [./rhouA_big]
    type = PointValue
    variable = rhouA
    point = '1.001 0 0'
  [../]
  [./test_rel_err]
    type = RelativeDifferencePostprocessor
    value1 = rhouA_big
    value2 = rhouA_small
  [../]
[]

[Outputs]
  [./out]
    type = CSV
    show = test_rel_err
  [../]
[]
