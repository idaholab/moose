cp_water_multiplier = 5e-2
mu_multiplier = 1

# Real facility uses forced convection to cool the water tank at full power
# Need to lower power for natural convection so concrete doesn't get too hot.
power = '${fparse 5e4 / 144 / 2}'

# Coupling
h_water = 600

[GlobalParams]
  # This parameter is used in numerous objects. It is often
  # best to define it here to avoid missing it in an object
  displacements = 'disp_x disp_y'
[]


[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../step11_multiapps/mesh2d_coarse_in.e'
  []
[]

[Problem]
  nl_sys_names = 'nl0 flow'
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Physics]
  [HeatConduction]
    [FiniteElement]
      [concrete]
        block = 'concrete_hd concrete Al'
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

        heat_flux_boundaries = 'inner_cavity_solid'
        # 50 kW from radiation, using real surface
        boundary_heat_fluxes = '${power}'

        fixed_convection_boundaries = "water_boundary_inwards air_boundary"
        fixed_convection_T_fluid = "T_fluid 300"
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
        block = 'concrete_hd concrete Al'
      []
    []
  []
  [NavierStokes]
    [Flow]
      [water]
        block = 'water'
        system_names = 'flow'
        compressibility = 'incompressible'

        initial_velocity = '1e-5 1e-5'
        initial_pressure = '1e5'

        # p only appears in a gradient term, and thus could be offset by any constant
        # We pin the pressure to avoid having this nullspace
        pin_pressure = true
        pinned_pressure_type = POINT-VALUE
        pinned_pressure_point = '1 3 0'
        pinned_pressure_value = '1e5'

        gravity = '0 -9.81 0'
        boussinesq_approximation = true
        ref_temperature = 300

        wall_boundaries = 'water_boundary inner_cavity_water'
        momentum_wall_types = 'noslip noslip'
      []
    []
    [FluidHeatTransfer]
      [water]
        block = 'water'
        system_names = 'flow'

        initial_temperature = 300

        # This is a rough coupling to heat conduction
        energy_wall_types = 'convection heatflux'
        energy_wall_functors = 'T:${h_water} ${power}'

        energy_scaling = 1e-5
      []
    []
    [Turbulence]
      [mixing-length]
        block = 'water'
        turbulence_handling = 'mixing-length'
        coupled_flow_physics = 'water'
        fluid_heat_transfer_physics = 'water'
        system_names = 'flow'
        mixing_length_walls = 'water_boundary inner_cavity_water'
        mixing_length_aux_execute_on = 'initial'
      []
    []
  []
[]

# These terms are not part of any Physics, yet!
[Kernels]
  [gravity]
    type = ADGravity
    variable = 'disp_y'
    value = '-9.81'
    block = 'concrete_hd concrete Al'
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
  [concrete_hd]
    type = ADHeatConductionMaterial
    block = 'concrete_hd'
    temp = 'T'
    # we specify a function of time, temperature is passed as the time argument
    # in the material
    thermal_conductivity_temperature_function = '5.0 + 0.001 * t'
  []
  [concrete]
    type = ADHeatConductionMaterial
    block = 'concrete'
    temp = 'T'
    thermal_conductivity_temperature_function = '2.25 + 0.001 * t'
  []
  [Al]
    type = ADHeatConductionMaterial
    block = 'Al'
    temp = T
    thermal_conductivity_temperature_function = '175'
  []

  # NOTE: This handles thermal expansion by coupling to the displacements
  [density_concrete_hd]
    type = ADDensity
    block = 'concrete_hd'
    density = '3524' # kg / m3
  []
  [density_concrete]
    type = ADDensity
    block = 'concrete'
    density = '2403' # kg / m3
  []
  [density_Al]
    type = ADDensity
    block = 'Al'
    density = '2270' # kg / m3
  []

  # Materials for solid mechanics
  [elasticity_tensor_concrete_hd]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 2.75e9 # (Pa)
    poissons_ratio = 0.15
    block = 'concrete_hd'
  []
  [elasticity_tensor_concrete]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 30e9 # (Pa)
    poissons_ratio = 0.2
    block = 'concrete'
  []
  [elasticity_tensor_Al]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 68e9 # (Pa)
    poissons_ratio = 0.36
    block = 'Al'
  []
  [elastic_stress]
    type = ADComputeFiniteStrainElasticStress
    block = 'concrete_hd concrete Al'
  []
  [thermal_strain_concrete_hd]
    type = ADComputeThermalExpansionEigenstrain
    stress_free_temperature = 300
    eigenstrain_name = eigenstrain
    temperature = T
    thermal_expansion_coeff = 1e-5 # 1/K
    block = 'concrete_hd'
  []
  [thermal_strain_concrete]
    type = ADComputeThermalExpansionEigenstrain
    stress_free_temperature = 300
    eigenstrain_name = eigenstrain
    temperature = T
    thermal_expansion_coeff = 1e-5 # 1/K
    block = 'concrete'
  []
  [thermal_strain_Al]
    type = ADComputeThermalExpansionEigenstrain
    stress_free_temperature = 300 # arbitrary value
    eigenstrain_name = eigenstrain
    temperature = T
    thermal_expansion_coeff = 2.4e-5 # 1/K
    block = 'Al'
  []
[]

[Executioner]
  type = Transient

  # Time stepping parameters
  start_time = -1
  end_time = ${units 4 h -> s}
  dtmax = 100
  [TimeStepper]
    type = FunctionDT
    function = 'if(t<0.1, 0.1, t)'
  []

  # Solver parameters
  solve_type = NEWTON
  automatic_scaling = true
  off_diagonals_in_auto_scaling = true
  line_search = none

  # Tolerances
  # Navier Stokes natural circulation will only converge so far
  nl_abs_tol = 6e-7
  nl_max_its = 15
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
  exodus = true
[]

[Postprocessors]
  # Useful information
  [T_fluid_average]
    type = ElementAverageValue
    variable = 'T_fluid'
    block = 'water'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T_solid_average]
    type = ElementAverageValue
    variable = 'T'
    block = 'concrete_hd concrete'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_dispx]
    type = ElementExtremeValue
    variable = 'disp_x'
    value_type = 'max_abs'
    block = 'concrete_hd concrete'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_dispy]
    type = ElementExtremeValue
    variable = 'disp_y'
    value_type = 'max_abs'
    block = 'concrete_hd concrete'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_Tsolid]
    type = ElementExtremeValue
    variable = 'T'
    block = 'concrete_hd concrete'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_Tfluid]
    type = ElementExtremeValue
    variable = 'T_fluid'
    block = 'water'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
