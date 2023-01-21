# This test tests conservation of mass, momentum, and energy on a transient
# problem with an inlet and outlet (using free boundaries for each). This test
# takes 1 time step with Crank-Nicolson and some boundary flux integral
# post-processors needed for the full conservation statement. Lastly, the
# conservation quantities are shown on the console, which should ideally be zero
# for full conservation.

[GlobalParams]
  gravity_vector = '0 0 0'

  scaling_factor_1phase = '1 1 1e-6'

  closures = simple_closures
[]

[Functions]
  [T_fn]
    type = ParsedFunction
    expression = '300 + 10 * (cos(2*pi*x + pi))'
  []
[]

[FluidProperties]
  [fp]
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
  [inlet]
    type = FreeBoundary1Phase
    input = pipe:in
  []
  [pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 10
    A = 1.0

    initial_T = T_fn
    initial_p = 1e5
    initial_vel = 1

    f = 0

    fp = fp
  []
  [outlet]
    type = FreeBoundary1Phase
    input = pipe:out
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
  scheme = crank-nicolson
  start_time = 0.0
  end_time = 0.01
  dt = 0.01
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-4
  nl_max_its = 10

  l_tol = 1e-2
  l_max_its = 20
[]

[Postprocessors]
  # MASS

  [massflux_left]
    type = MassFluxIntegral
    boundary = inlet
    arhouA = rhouA
  []
  [massflux_right]
    type = MassFluxIntegral
    boundary = outlet
    arhouA = rhouA
  []
  [massflux_difference]
    type = DifferencePostprocessor
    value1 = massflux_right
    value2 = massflux_left
  []
  [massflux_integral]
    type = TimeIntegratedPostprocessor
    value = massflux_difference
  []
  [mass]
    type = ElementIntegralVariablePostprocessor
    variable = rhoA
    execute_on = 'initial timestep_end'
  []
  [mass_change]
    type = ChangeOverTimePostprocessor
    postprocessor = mass
    change_with_respect_to_initial = true
    execute_on = 'initial timestep_end'
  []
  [mass_conservation]
    type = SumPostprocessor
    values = 'mass_change massflux_integral'
  []

  # MOMENTUM

  [momentumflux_left]
    type = MomentumFluxIntegral
    boundary = inlet
    arhouA = rhouA
    vel = vel
    p = p
    A = A
  []
  [momentumflux_right]
    type = MomentumFluxIntegral
    boundary = outlet
    arhouA = rhouA
    vel = vel
    p = p
    A = A
  []
  [momentumflux_difference]
    type = DifferencePostprocessor
    value1 = momentumflux_right
    value2 = momentumflux_left
  []
  [momentumflux_integral]
    type = TimeIntegratedPostprocessor
    value = momentumflux_difference
  []
  [momentum]
    type = ElementIntegralVariablePostprocessor
    variable = rhouA
    execute_on = 'initial timestep_end'
  []
  [momentum_change]
    type = ChangeOverTimePostprocessor
    postprocessor = momentum
    change_with_respect_to_initial = true
    execute_on = 'initial timestep_end'
  []
  [momentum_conservation]
    type = SumPostprocessor
    values = 'momentum_change momentumflux_integral'
  []

  # ENERGY

  [energyflux_left]
    type = EnergyFluxIntegral
    boundary = inlet
    arhouA = rhouA
    H = H
  []
  [energyflux_right]
    type = EnergyFluxIntegral
    boundary = outlet
    arhouA = rhouA
    H = H
  []
  [energyflux_difference]
    type = DifferencePostprocessor
    value1 = energyflux_right
    value2 = energyflux_left
  []
  [energyflux_integral]
    type = TimeIntegratedPostprocessor
    value = energyflux_difference
  []
  [energy]
    type = ElementIntegralVariablePostprocessor
    variable = rhoEA
    execute_on = 'initial timestep_end'
  []
  [energy_change]
    type = ChangeOverTimePostprocessor
    postprocessor = energy
    change_with_respect_to_initial = true
    execute_on = 'initial timestep_end'
  []
  [energy_conservation]
    type = SumPostprocessor
    values = 'energy_change energyflux_integral'
  []
[]

[Outputs]
  [console]
    type = Console
    show = 'mass_conservation momentum_conservation energy_conservation'
  []
  velocity_as_vector = false
[]
