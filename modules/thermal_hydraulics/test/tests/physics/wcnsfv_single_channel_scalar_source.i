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
      [flow]
        fp = fp
      []
    []
    [WCNSFVScalar]
      [scalar]
        passive_scalar_names = 'c1 c2'
        fp = fp
      []
    []
  []
[]

[Components]
  [inlet]
    type = PhysicsGeneralFlowBoundary
    input = 'pipe:in'
    fixed_values_variables = 'T_fluid c1 c2'
    fixed_values_functors = '500 0.1 0.01'
    # the mass equation is the equation for the pressure in WCNSFV
    # the flux for the mass equation is the mass flow rate
    fixed_equation_flux_variables = 'pressure'
    fixed_equation_flux_functors = '1'
  []

  [pipe]
    type = PhysicsFlowChannel

    # pipe geometry and discretization
    position = '0 0 0'
    orientation = '0 1 0'
    gravity_vector = '-9000 0 0'
    length = 1.0
    n_elems = 5
    A = 1.0

    initial_T = 300
    initial_p = 1e5
    initial_vel = 1
    initial_scalars = '1 2'

    physics = 'flow scalar'
    f = 1e10
    closures = simple_closures
    fp = fp
  []

  # Uses the functors to add / remove some scalar
  [scalar_transfer]
    type = PhysicsScalarTransferFromFunctors
    flow_channel = pipe
    passive_scalar_names = 'c1 c2'
    wall_scalar_values = '1 0.1'
    H = '1 10'
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

[AuxVariables]
  [mc1]
    type = MooseVariableFVReal
  []
  [mc2]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [c1_specific]
    type = ParsedAux
    variable = mc1
    expression = 'c1 / rho'
    functor_names = 'rho c1'
  []
  [c2_specific]
    type = ParsedAux
    variable = mc2
    expression = 'c2 / rho'
    functor_names = 'rho c2'
  []
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
  # wont match 'vel_right' because of density difference
  [vel_left]
    type = SideAverageValue
    variable = 'vel_x'
    boundary = 'pipe:out'
  []
  [c1_left]
    type = SideAverageValue
    variable = 'c1'
    boundary = 'pipe:in'
  []
  [c2_left]
    type = SideAverageValue
    variable = 'c2'
    boundary = 'pipe:in'
  []
  [c1_right]
    type = SideAverageValue
    variable = 'c1'
    boundary = 'pipe:out'
  []
  [c2_right]
    type = SideAverageValue
    variable = 'c2'
    boundary = 'pipe:out'
  []
[]
