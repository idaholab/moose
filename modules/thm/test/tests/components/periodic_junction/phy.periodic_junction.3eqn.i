# This test problem tests the PeriodicJunction component, which supplies
# periodic BC linking pipes. This test is an abridged version of the periodic
# junction test problem in tests/problems/periodic_junction; the full problem,
# which makes a full revolution around the domain requires too many time steps,
# and testing long transients are sensitive to accumulation of differences
# between architectures. Therefore, this test takes only a few time steps, uses
# a smaller number of elements, and uses only one pipe. This test problem
# consists of steam with an initially sinusoidal temperature profile that
# advects at a constant speed with no external forces.

[GlobalParams]
  scaling_factor_1phase = '1e0 1e-3 1e-5'

  gravity_vector = '0 0 0'

  closures = simple

  spatial_discretization = cg
[]

[FluidProperties]
  [./eos]
    type = StiffenedGasFluidProperties
    gamma = 1.43
    cv = 1040.0
    q = 2.03e6
    p_inf = 0.0
    q_prime = -2.3e4
  [../]
[]

[Functions]
  [./T_fn]
    type = ParsedFunction
    value = '500 + 10*(cos(2*pi*x + pi) + 1)'
  [../]
[]
[Components]
  [./pipe]
    type = FlowChannel1Phase
    fp = eos

    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 10
    A = 1.0

    initial_p = 1e5
    initial_T = T_fn
    initial_vel = 1

    f = 0
  [../]
  [./junction]
    type = PeriodicJunction
    connections = 'pipe:out pipe:in'
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

  start_time = 0.0
  end_time = 0.1
  [./TimeStepper]
    type = ConstantDT
    dt = 0.01
  [../]
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 10

  [./Quadrature]
    type = GAUSS
    order = SECOND
  [../]
[]

[Outputs]
  file_base = 'phy.periodic_junction.3eqn'
  [./out]
    type = Exodus
  [../]
[]
