# Speeds up the transient if < 1
cp_water_multiplier = 1e-5
# Makes the problem more diffusive if > 1
mu_multiplier = 1e0

h_water = 30

[GlobalParams]
  # This parameter is used in numerous objects. It is often
  # best to define it here to avoid missing it in an object
  displacements = 'disp_x disp_y disp_z'
[]


[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../step03_boundary_conditions/mesh_in.e'
  []
  [add_inner_water]
    type = SideSetsFromBoundingBoxGenerator
    input = fmg
    included_boundaries = 'water_boundary'
    boundary_new = water_boundary_inner
    bottom_left = '2.5 2.5 1'
    top_right = '6.6 10.5 5'
    location = INSIDE
  []
  [add_outer_water]
    type = SideSetsFromBoundingBoxGenerator
    input = add_inner_water
    included_boundaries = 'water_boundary'
    boundary_new = water_boundary_outer
    bottom_left = '2.5 2.5 1'
    top_right = '6.6 10.5 5'
    location = OUTSIDE
  []
  # careful, this is not enough refinement for a converged result
  [refine_water]
    type = RefineBlockGenerator
    input = add_outer_water
    refinement = '1'
    block = 'water'
  []
[]

[Problem]
  nl_sys_names = 'nl0 flow'
[]

[Physics]
  [HeatConduction]
    [FiniteElement]
      [concrete]
        block = 'concrete concrete_and_Al'
        temperature_name = "T"
        system_names = 'nl0'
        preconditioning = 'none'

        # Solve for steady state
        # It takes a while to heat up concrete
        initial_temperature = 300
        transient = false

        # Heat conduction boundary conditions can be defined
        # inside the HeatConduction physics block
        fixed_temperature_boundaries = 'ground'
        boundary_temperatures = '300'

        heat_flux_boundaries = 'inner_cavity'
        # 50 kW from radiation, using real surface
        boundary_heat_fluxes = '${fparse 5e4 / 136}'

        fixed_convection_boundaries = "water_boundary_inwards air_boundary"
        # TODO: enable using a field instead of postprocessor
        fixed_convection_T_fluid = "T_fluid_average 300"
        # Note: should come from a correlation
        fixed_convection_htc = "${h_water} 10"
      []
    []
  []
  [SolidMechanics]
    [QuasiStatic]
      [concrete]
        # This block adds all of the proper Kernels, strain calculators, and Variables
        # for Solid Mechanics in the correct coordinate system (autodetected)
        add_variables = true
        strain = FINITE
        eigenstrain_names = eigenstrain
        use_automatic_differentiation = true
        generate_output = 'vonmises_stress elastic_strain_xx elastic_strain_yy strain_xx strain_yy'
        block = 'concrete concrete_and_Al'
      []
    []
  []
  [NavierStokes]
    [Flow]
      [water]
        block = 'water'
        system_names = 'flow'
        compressibility = 'incompressible'

        initial_velocity = '1e-5 1e-5 1e-5'
        initial_pressure = '1e5'

        # p only appears in a gradient term, and thus could be offset by any constant
        # We pin the pressure to avoid having this nullspace
        pin_pressure = true
        pinned_pressure_type = 'point-value'
        pinned_pressure_point = '2 2 2'
        pinned_pressure_value = '1e5'

        gravity = '0 0 -9.81'
        boussinesq_approximation = true
        ref_temperature = 300

        wall_boundaries = 'water_boundary'
        momentum_wall_types = 'noslip'
      []
    []
    [FluidHeatTransfer]
      [water]
        block = 'water'
        system_names = 'flow'

        initial_temperature = 300

        # This is a rough coupling to heat conduction
        energy_wall_types = 'heatflux'
        energy_wall_functors = 'h_dT'

        energy_scaling = 1e-5
      []
    []
  []
[]

# These terms are not part of any Physics, yet!
[Kernels]
  [gravity]
    type = ADGravity
    variable = 'disp_z'
    value = '-9.81'
    block = 'concrete concrete_and_Al'
  []
[]
# TODO: add volumetric heat deposition, either in the Kernels or Physics blocks

# The solid mechanics boundary conditions are defined outside the physics
[BCs]
  [hold_ground_x]
    type = DirichletBC
    variable = disp_x
    boundary = ground
    value = 0
  []
  [hold_ground_y]
    type = DirichletBC
    variable = disp_y
    boundary = ground
    value = 0
  []
  [hold_ground_z]
    type = DirichletBC
    variable = disp_z
    boundary = ground
    value = 0
  []
[]

[FunctorMaterials]
  # Materials for fluid flow
  [water]
    type = ADGenericFunctorMaterial
    block = 'water'
    prop_names = 'rho    k     cp      mu alpha_wall alpha'
    prop_values = '955.7 0.6 ${fparse cp_water_multiplier * 4181} ${fparse 7.98e-4 * mu_multiplier} 30 2e-4'
  []

  # Boundary condition functor computing the heat flux with a set heat transfer coefficient
  [heat-flux]
    type = ADParsedFunctorMaterial
    expression = '${h_water} * (T - T_fluid)'
    functor_names = 'T T_fluid'
    property_name = 'h_dT'
  []
[]

[Materials]
  # Materials for heat conduction
  [concrete_thermal]
    type = ADHeatConductionMaterial
    block = 'concrete'
    temp = 'T'
    # we specify a function of time, temperature is passed as the time argument
    # by the material
    thermal_conductivity_temperature_function = '2.25 + 0.001 * t'
    specific_heat = '1170'
  []
  [concrete_and_Al]
    type = ADHeatConductionMaterial
    block = 'concrete_and_Al'
    temp = 'T'
    # Al: 175 W/m/K, concrete: 2.5 W/m/K
    thermal_conductivity_temperature_function = '45'
    specific_heat = '1170'
  []
  # NOTE: This handles thermal expansion by coupling to the displacements
  [concrete_density]
    type = ADDensity
    density = '2400'
    block = 'concrete concrete_and_Al'
  []

  # Materials for solid mechanics
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 200e9 # (Pa) from wikipedia
    poissons_ratio = .3 # from wikipedia
    block = 'concrete concrete_and_Al'
  []
  [elastic_stress]
    type = ADComputeFiniteStrainElasticStress
    block = 'concrete concrete_and_Al'
  []
  [thermal_strain]
    type = ADComputeThermalExpansionEigenstrain
    stress_free_temperature = 300
    eigenstrain_name = eigenstrain
    temperature = "T"
    # Increased to have a more dramatic expansion
    thermal_expansion_coeff = 1e-5
    block = 'concrete concrete_and_Al'
  []
[]

[Executioner]
  type = Transient

  # Time stepping parameters
  start_time = -1
  end_time = 200
  [TimeStepper]
    type = FunctionDT
    function = 'if(t<0,0.1,0.25)'
  []

  # Let it develop
  steady_state_start_time = 5
  steady_state_tolerance = 1e-7
  steady_state_detection = true

  # Solver parameters
  solve_type = NEWTON
  automatic_scaling = true
  line_search = none

  # Tolerances
  # Navier Stokes natural circulation will only converge so far
  nl_abs_tol = 6e-7
[]

[Preconditioning]
  [thermomecha]
    type = SMP
    nl_sys = 'nl0'
    petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
    petsc_options_value = 'hypre boomeramg 500'
  []
  [flow]
    type = SMP
    nl_sys = 'flow'
    petsc_options_iname = '-pc_type -pc_factor_shift_type'
    petsc_options_value = 'lu NONZERO'
  []
[]

[Outputs]
  csv = true
  [out]
    type = Exodus
    use_displaced = true
  []
[]


[Postprocessors]
  [T_fluid_average]
    type = ElementAverageValue
    variable = 'T_fluid'
    block = 'water'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T_solid_average]
    type = ElementAverageValue
    variable = 'T'
    block = 'concrete concrete_and_Al'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_dispx]
    type = ElementExtremeValue
    variable = 'disp_x'
    value_type = 'max_abs'
    block = 'concrete concrete_and_Al'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_dispy]
    type = ElementExtremeValue
    variable = 'disp_y'
    value_type = 'max_abs'
    block = 'concrete concrete_and_Al'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_dispz]
    type = ElementExtremeValue
    variable = 'disp_z'
    value_type = 'max_abs'
    block = 'concrete concrete_and_Al'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_Tsolid]
    type = ElementExtremeValue
    variable = 'T'
    block = 'concrete concrete_and_Al'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
