# Tests that a flow channel can run with Steady executioner and be set up using Physics
#
# Note that this solve may fail to converge based on initial guess. For example,
# having a guess with velocity set to zero will fail to converge.

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
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
        fp = fp
        velocity_interpolation = 'average'

        dynamic_viscosity = 0
      []
    []
  []
[]

[Components]
  [inlet]
    type = PhysicsInletMassFlowRateTemperature
    input = 'pipe:in'
    m_dot = 2
    T = 500
  []

  [pipe]
    type = PhysicsFlowChannel
    position = '0 0 0'
    orientation = '0 1 0'
    gravity_vector = '-9000 0 0'
    length = 1.0
    n_elems = 5
    A = 1.0

    initial_T = 300
    initial_p = 1e5
    initial_vel = 1

    physics = 'all'
    f = 1e10
    closures = simple_closures
    fp = fp
  []

  [outlet]
    type = PhysicsOutlet
    input = 'pipe:out'
    p = 2e5
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

  num_steps = 3

  solve_type = NEWTON
  nl_rel_tol = 1e-7
  nl_abs_tol = 1e-7
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 10

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Outputs]
  exodus = true
[]


[Postprocessors]
  [pressure_left]
    type = SideAverageValue
    variable = pressure
    boundary = pipe:out
    execute_on = 'initial timestep_end'
  []
  [pressure_right]
    type = SideAverageValue
    variable = pressure
    boundary = pipe:in
    execute_on = 'initial timestep_end'
  []
  [mass_right]
    type = VolumetricFlowRate
    boundary = pipe:out
    vel_x = vel_x
    advected_quantity = 'rho'
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    execute_on = 'initial timestep_end'
  []
  [vel_right]
    type = SideAverageValue
    variable = 'vel_x'
    boundary = 'pipe:in'
  []
  # wont match because of pressure difference
  [vel_left]
    type = SideAverageValue
    variable = 'vel_x'
    boundary = 'pipe:out'
  []
[]
