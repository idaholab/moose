# This test problem tests the PeriodicJunction component, which supplies
# periodic BC linking pipes. This test is an abridged version of the periodic
# junction test problem in tests/problems/periodic_junction; the full problem,
# which makes a full revolution around the domain requires too many time steps,
# and testing long transients are sensitive to accumulation of differences
# between architectures. Therefore, this test takes only a few time steps, uses
# a smaller number of elements, and uses only one pipe. This test problem
# consists of steam/liquid mixture with an initially sinusoidal temperature profile
# in both phases that are advected at a constant speed with no external forces.

[GlobalParams]
  scaling_factor_2phase = '1e0 1e0 1e-2 1e-4 1e0 1e-3 1e-5'

  gravity_vector = '0 0 0'
  pressure_relaxation = false
  velocity_relaxation = false
  interface_transfer = false
  wall_mass_transfer = false

  specific_interfacial_area_max_value = 10
  explicit_acoustic_impedance = true
  explicit_alpha_gradient = true

  closures = simple
[]

[FluidProperties]
  [./eos]
    type = StiffenedGasTwoPhaseFluidProperties
  [../]
[]

[Functions]
  [./T_liquid_fn]
    type = ParsedFunction
    value = '300 + 10*(cos(2*pi*x + pi) + 1)'
  [../]
  [./T_vapor_fn]
    type = ParsedFunction
    value = '500 + 10*(cos(2*pi*x + pi) + 1)'
  [../]
[]
[Components]
  [./pipe]
    type = FlowChannel2Phase
    fp = eos

    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 10
    A = 1.0

    initial_p_liquid = 1e5
    initial_p_vapor = 1e5
    initial_T_liquid = T_liquid_fn
    initial_T_vapor = T_vapor_fn
    initial_vel_liquid = 1
    initial_vel_vapor = 1
    initial_alpha_vapor = 0.2

    f = 0
    f_interface = 0
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

  solve_type = 'PJFNK'
  line_search = 'basic'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  nl_max_its = 20

  l_tol = 1e-2
  l_max_its = 10

  [./Quadrature]
    type = GAUSS
    order = SECOND
  [../]
[]

[Outputs]
  file_base = 'phy.periodic_junction.7eqn'
  exodus = true
[]
