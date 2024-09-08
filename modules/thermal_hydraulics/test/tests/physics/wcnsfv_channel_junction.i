# This input file simulates the Sod shock tube using a junction in the middle
# of the domain. The solution should be exactly equivalent to the problem with
# no junction. This test examines the solutions at the junction connections
# and compares them to gold values generated from a version of this input file
# that has no junction.

T_inlet = 300
T_init = 300
p_outlet = 2e5

[GlobalParams]
  gravity_vector = '0 0 0'

  closures = simple_closures
[]

[Functions]
  [p_ic_fn]
    type = PiecewiseConstant
    axis = x
    direction = right
    x = '0.5 1.0'
    y = '${p_outlet} ${p_outlet}'
  []

  [T_ic_fn]
    type = PiecewiseConstant
    axis = x
    direction = right
    x = '0.5 1.0'
    y = '${T_inlet} ${T_init}'
  []
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 11.64024372
  []
[]

[FunctorMaterials]
  [functor_fluid_props]
    type = GeneralFunctorFluidProps
    fp = fp
    T_fluid = 500
    pressure = 'pressure'
    characteristic_length = 1
    porosity = 1
    speed = 1 # Re unused
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Physics]
  [ThermalHydraulics]
    [WCNSFV]
      [all]
        # needed to avoid ANY_BLOCK_ID
        block = 'left_channel'
        fp = fp
        verbose = true
      []
    []
  []
[]


[Components]
  [left_boundary]
    type = PhysicsInletMassFlowRateTemperature
    input = 'left_channel:in'
    m_dot = 2
    T = ${T_inlet}
  []

  [left_channel]
    type = PhysicsFlowChannel

    physics = 'all'
    fp = fp

    position = '0 0 0'
    orientation = '1 0 0'
    length = 0.5
    n_elems = 50
    A = 1.0

    initial_T = T_ic_fn
    initial_p = p_ic_fn
    initial_vel = 0.1

    f = 0
  []

  [junction]
    type = PhysicsJunctionOneToOne
    # this order matters for the problem setup
    connections = 'left_channel:out right_channel:in'
  []

  [right_channel]
    type = PhysicsFlowChannel

    physics = 'all'
    fp = fp

    position = '0.5 0 0'
    orientation = '1 0 0'
    length = 0.5
    n_elems = 50
    A = 1.0

    initial_T = T_ic_fn
    initial_p = p_ic_fn
    initial_vel = 0.1

    f = 0
  []

  [right_boundary]
    type = PhysicsOutlet
    input = 'right_channel:out'
    p = ${p_outlet}
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2

  petsc_options_iname = '-pc_type -pc_factor_shift'
  petsc_options_value = 'lu nonzero'

  solve_type = NEWTON
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  nl_max_its = 60

  l_tol = 1e-4

  start_time = 0.0
  dt = 1e-3
  num_steps = 5
  abort_on_solve_fail = true
[]

[Postprocessors]
  [pressure_left]
    type = SideAverageValue
    variable = pressure
    boundary = left_channel:out
    execute_on = 'initial timestep_end'
  []
  [vel_x_left]
    type = SideAverageValue
    variable = vel_x
    boundary = left_channel:out
    execute_on = 'initial timestep_end'
  []
  [pressure_right]
    type = SideAverageValue
    variable = pressure
    boundary = right_channel:in
    execute_on = 'initial timestep_end'
  []
  [vel_x_right]
    type = SideAverageValue
    variable = vel_x
    boundary = right_channel:in
    execute_on = 'initial timestep_end'
  []

  # This is present to test that junction sidesets work properly
  [p_avg_junction]
    type = SideAverageValue
    boundary = 'junction'
    variable = pressure
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
  execute_on = 'initial nonlinear timestep_end'
[]
