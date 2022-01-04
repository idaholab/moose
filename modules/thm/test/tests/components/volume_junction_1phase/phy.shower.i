# This problem models a "shower": water from two pipes, one hot and one cold,
# mixes together to produce a temperature between the two.

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T = 300
  initial_p = 1e5
  initial_vel = 0
  initial_vel_x = 0
  initial_vel_y = 0
  initial_vel_z = 0

  # global parameters for pipes
  fp = eos
  orientation = '1 0 0'
  length = 1
  n_elems = 20
  f = 0

  scaling_factor_1phase = '1 1 1e-6'

  closures = simple_closures
[]

[FluidProperties]
  [eos]
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

[Components]
  [inlet_hot]
    type = InletDensityVelocity1Phase
    input = 'pipe_hot:in'
    # rho @ (p = 1e5, T = 310 K)
    rho = 1315.9279785683
    vel = 1
  []
  [inlet_cold]
    type = InletDensityVelocity1Phase
    input = 'pipe_cold:in'
    # rho @ (p = 1e5, T = 280 K)
    rho = 1456.9202619863
    vel = 1
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe_warm:out'
    p = 1e5
  []

  [pipe_hot]
    type = FlowChannel1Phase
    position = '0 1 0'
    A = 1
  []
  [pipe_cold]
    type = FlowChannel1Phase
    position = '0 0 0'
    A = 1
  []
  [pipe_warm]
    type = FlowChannel1Phase
    position = '1 0.5 0'
    A = 2
  []

  [junction]
    type = VolumeJunction1Phase
    connections = 'pipe_cold:out pipe_hot:out pipe_warm:in'
    position = '1 0.5 0'
    volume = 1e-8
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
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-5
  nl_max_its = 10

  l_tol = 1e-2
  l_max_its = 10

  start_time = 0
  end_time = 5
  dt = 0.05

  # abort_on_solve_fail = true
[]

[Postprocessors]
  # These post-processors are used to test that the energy flux on
  # the warm side of the junction is equal to the sum of the energy
  # fluxes of the hot and cold inlets to the junction.
  [energy_flux_hot]
    type = EnergyFluxIntegral
    boundary = pipe_hot:out
    arhouA = rhouA
    H = H
  []
  [energy_flux_cold]
    type = EnergyFluxIntegral
    boundary = pipe_cold:out
    arhouA = rhouA
    H = H
  []
  [energy_flux_warm]
    type = EnergyFluxIntegral
    boundary = pipe_warm:in
    arhouA = rhouA
    H = H
  []
  [energy_flux_inlet_sum]
    type = SumPostprocessor
    values = 'energy_flux_hot energy_flux_cold'
  []
  [test_rel_err]
    type = RelativeDifferencePostprocessor
    value1 = energy_flux_warm
    value2 = energy_flux_inlet_sum
  []
[]

[Outputs]
  [out]
    type = CSV
    show = test_rel_err
    sync_only = true
    sync_times = '3 4 5'
  []
  [console]
    type = Console
    max_rows = 1
  []
  print_linear_residuals = false
[]
