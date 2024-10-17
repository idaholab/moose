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
    [WeaklyCompressibleFlow]
      [flow]
        fp = fp
      []
    []

    [WeaklyCompressibleScalar]
      [scalar]
        passive_scalar_names = 'c1 c2'
        fp = fp
      []
    []
  []
  [Diffusion]
    [ContinuousGalerkin]
      [diff_1]
        variable_name = 'u1'
        diffusivity_matprop = '3'
        source_functor = '1'

        # to avoid defining it everywhere
        block = ''

        # we specify boundary conditions here rather than on the PhysicsFileMeshComponent
        # because that component does not allow setting boundary conditions
        dirichlet_boundaries = 'scalar_structure:left scalar_structure:right'
        boundary_values = '1 2'
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
    position = '1 1 0'
    orientation = '1 0 0'
    gravity_vector = '-9000 0 0'
    length = 1.0
    # keep higher than number of layered averages
    n_elems = 10
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

  # Uses the functors defined by the materials extracting the values from the file mesh physics component
  [scalar_transfer]
    type = PhysicsScalarTransferFromFunctors
    flow_channel = pipe
    passive_scalar_names = 'c1 c2'
    wall_scalar_values = 'from_struct_u1 3'
    H = '1 10'
  []

  # A non-thermal hydraulics physics is being solved on component
  [scalar_structure]
    type = FileMeshPhysicsComponent
    physics = diff_1
    file = 'mesh/rectangle_in.e'
    # shift the mesh to line up with the component
    position = '1 1 0'
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

[UserObjects]
  # in this example, we use layered averages to get the boundary value of the component
  # as if the component was wrapping around the flow channel
  [layer_average_u1]
    type = LayeredAverage
    block = 'scalar_structure:0'
    variable = u1
    num_layers = 10
    direction = 'x'
  []
[]

[FunctorMaterials]
  [average_u1]
    type = SpatialUserObjectFunctorMaterial
    user_object = layer_average_u1
    functor_name = from_struct_u1
  []
[]

[Outputs]
  exodus = true
[]

[AuxVariables]
  [mc1]
    type = MooseVariableFVReal
    block = 'pipe'
  []
  [mc2]
    type = MooseVariableFVReal
    block = 'pipe'
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
  [mc1_left]
    type = SideAverageValue
    variable = 'mc1'
    boundary = 'pipe:in'
  []
  [mc2_left]
    type = SideAverageValue
    variable = 'mc2'
    boundary = 'pipe:in'
  []
  [mc1_right]
    type = SideAverageValue
    variable = 'mc1'
    boundary = 'pipe:out'
  []
  [mc2_right]
    type = SideAverageValue
    variable = 'mc2'
    boundary = 'pipe:out'
  []
[]
