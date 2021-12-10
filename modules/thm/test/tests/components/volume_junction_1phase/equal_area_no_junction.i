# Tests a junction between 2 flow channels of equal area and orientation. A
# sinusoidal density shape is advected to the right and should not be affected
# by the junction; the solution should be identical to the equivalent
# no-junction solution.
#
# This input file has no junction and is used for comparison to the results with
# a junction.

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 1e5
  initial_vel = 1

  A = 25

  f = 0

  fp = fp

  scaling_factor_1phase = '0.04 0.04 0.04e-5'

  closures = simple_closures
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 1.4
    cv = 725
    p_inf = 0
    q = 0
    q_prime = 0
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Functions]
  [T0]
    type = CosineHumpFunction
    axis = x
    hump_center_position = 1
    hump_width = 0.5
    hump_begin_value = 250
    hump_center_value = 300
  []
[]

[Components]
  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe1:in'
    # Stagnation property with p = 1e5 Pa, T = 250 K, vel = 1 m/s
    p0 = 100000.68965687
    T0 = 250.00049261084
  []

  [pipe1]
    type = FlowChannel1Phase

    position = '0 0 0'
    orientation = '1 0 0'
    length = 2

    initial_T = T0

    n_elems = 50
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe1:out'
    p = 1e5
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
  start_time = 0
  dt = 0.01
  num_steps = 5
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 0
  nl_abs_tol = 1e-6
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 10

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Postprocessors]
  [junction_rhoA]
    type = PointValue
    variable = rhoA
    point = '1.02 0 0'
    execute_on = 'initial timestep_end'
  []
  [junction_rhouA]
    type = PointValue
    variable = rhouA
    point = '1.02 0 0'
    execute_on = 'initial timestep_end'
  []
  [junction_rhoEA]
    type = PointValue
    variable = rhoEA
    point = '1.02 0 0'
    execute_on = 'initial timestep_end'
  []
  [junction_rho]
    type = ScalePostprocessor
    value = junction_rhoA
    scaling_factor = 0.04
    execute_on = 'initial timestep_end'
  []
  [junction_rhou]
    type = ScalePostprocessor
    value = junction_rhouA
    scaling_factor = 0.04
    execute_on = 'initial timestep_end'
  []
  [junction_rhoE]
    type = ScalePostprocessor
    value = junction_rhoEA
    scaling_factor = 0.04
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  [out]
    type = CSV
    show = 'junction_rho junction_rhou junction_rhoE'
    execute_scalars_on = 'none'
    execute_on = 'initial timestep_end'
  []
[]
