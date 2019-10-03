# Tests a junction between 2 pipes of equal area. A sinusoidal density shape is
# advected to the right and should not be affected by the junction; the solution
# should be identical to the equivalent no-junction solution.

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 1e5
  initial_T = 250

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

[Functions]
  [./T0]
    type = ParsedFunction
    value = 'if (x < 1, 300 + 50 * sin(2*pi*x + 1.5*pi), 250)'
  [../]
[]

[Components]
  [./inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe1:in'
    # Stagnation property with p = 1e5 Pa, T = 250 K, vel = 1 m/s
    p0 = 100000.68965687
    T0 = 250.00049261084
  [../]

  [./pipe1]
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

  [./junction]
    type = Junction
    connections = 'pipe1:out pipe2:in'
  [../]

  [./pipe2]
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
    input = 'pipe2:out'
    p = 1e5
  [../]
[]

[Preconditioning]
  [./pc]
    type = SMP
    full = true
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
  end_time = 0.5

  dt = 0.05
  abort_on_solve_fail = true

  [./Quadrature]
    type = GAUSS
    order = SECOND
  [../]
[]

[Outputs]
  [./out]
    type = Exodus
    execute_on = final
  [../]
[]
