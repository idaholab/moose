cp_water_multiplier = 1e3
mu_multiplier = 1e2

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
[]

[Problem]
  nl_sys_names = 'nl0'
[]

[Physics]
  [HeatConduction]
    [FiniteElement]
      [concrete]
        block = 'concrete'
        temperature_name = "T"
        system_names = 'nl0'

        # Solve for steady state
        # It takes a while to heat up concrete
        transient = false

        # Heat conduction boundary conditions can be defined
        # inside the HeatConduction physics block
        fixed_temperature_boundaries = 'ground'
        boundary_temperatures = '300'

        heat_flux_boundaries = 'inner_cavity'
        boundary_heat_fluxes = '${fparse 5e4 / 108}'

        fixed_convection_boundaries = "water_boundary_inwards air_boundary"
        fixed_convection_T_fluid = "T_fluid 300"
        # Note: should come from a correlation
        fixed_convection_htc = "30 10"
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
        block = 'concrete'
      []
    []
  []
  [NavierStokes]
    [Flow]
      [water]
        block = 'water'
        # system_names = 'flow'

        initial_velocity = '1e-15 1e-15 1e-15'

        # p only appears in a gradient term, and thus could be offset by any constant
        # We pin the pressure to avoid having this nullspace
        pin_pressure = true
        pinned_pressure_type = 'average'

        gravity = '0 0 -9.81'
        boussinesq_approximation = true
        # material property for reference temperature does not need to be AD material property
        ref_temperature = 300

        wall_boundaries = 'water_boundary'
        momentum_wall_types = 'noslip'
      []
    []
    [FluidHeatTransfer]
      [water]
        block = 'water'
        # system_names = 'flow'

        initial_temperature = 310

        # This is a rough coupling to heat conduction
        energy_wall_types = 'fixed-temperature'
        energy_wall_functors = 'T'
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
    block = 'concrete'
  []
[]

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
  # NOTE: This handles thermal expansion by coupling to the displacements
  [concrete_density]
    type = ADDensity
    density = '2400'
  []

  # Materials for solid mechanics
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 200e9 # (Pa) from wikipedia
    poissons_ratio = .3 # from wikipedia
    block = 'concrete'
  []
  [elastic_stress]
    type = ADComputeFiniteStrainElasticStress
    block = 'concrete'
  []
  [thermal_strain]
    type = ADComputeThermalExpansionEigenstrain
    stress_free_temperature = 300
    eigenstrain_name = eigenstrain
    temperature = "T"
    # Increased to have a more dramatic expansion
    thermal_expansion_coeff = 1e-4
    block = 'concrete'
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

  steady_state_tolerance = 1e-7
  steady_state_detection = true

  # Solver parameters
  solve_type = PJFNK
  automatic_scaling = true
  # petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  # petsc_options_value = 'hypre boomeramg 500'
  # Direct solve is useful for debugging convergence issues
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  line_search = none

  # Tolerances
  nl_abs_tol = 1e-11
[]

[Outputs]
  [out]
    type = Exodus
    use_displaced = true
  []
[]
