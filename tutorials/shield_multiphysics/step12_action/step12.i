cp_water_multiplier = 1e-4
mu_multiplier = 1e3

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../../step03_boundary_conditions/inputs/mesh_in.e'
  []
  [add_inner_water]
    type = SideSetsFromBoundingBoxGenerator
    input = fmg
    boundaries_old = 'water_boundary'
    boundary_new = water_boundary_inner
    bottom_left = '2.5 2.5 1'
    top_right = '6.6 10.5 5'
    location = INSIDE
  []
  [add_outer_water]
    type = SideSetsFromBoundingBoxGenerator
    input = add_inner_water
    boundaries_old = 'water_boundary'
    boundary_new = water_boundary_outer
    bottom_left = '2.5 2.5 1'
    top_right = '6.6 10.5 5'
    location = OUTSIDE
  []
[]

# TODO: Add an action for heat conduction
[Modules/IncompressibleNavierStokes]
  block = 'water'

  # TODO Complete this to set up natural convection in the water regions
  equation_type = steady-state

  gravity = '0 0 -9.81'

  velocity_boundary = 'water_boundary'
  velocity_function = '0 0 0'

  # Even though we are integrating by parts, because there are no integrated
  # boundary conditions on the velocity p doesn't appear in the system of
  # equations. Thus we must pin the pressure somewhere in order to ensure a
  # unique solution
  pressure_pinned_node = 2000

  density_name = rho
  dynamic_viscosity_name = mu

  initial_velocity = '1e-15 1e-15 1e-15'

  use_ad = true
  add_standard_velocity_variables_for_ad = false
  pspg = true
  supg = true

  family = LAGRANGE
  order = FIRST

  add_temperature_equation = true
  temperature_variable = temp
  initial_temperature = 340
  thermal_conductivity_name = k
  specific_heat_name = cp
  # natural_temperature_boundary = 'top bottom'
  fixed_temperature_boundary = 'water_boundary_outer water_boundary_inner'
  temperature_function = '300 400'

  boussinesq_approximation = true
  # material property for reference temperature does not need to be AD material property
  reference_temperature_name = 300
  thermal_expansion_name = alpha
[]

[AuxVariables]
  [temperature]
    block = 'concrete'
    initial_condition = 350
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    # This block adds all of the proper Kernels, strain calculators, and Variables
    # for TensorMechanics in the correct coordinate system (autodetected)
    add_variables = true
    strain = FINITE
    eigenstrain_names = eigenstrain
    use_automatic_differentiation = true
    generate_output = 'vonmises_stress elastic_strain_xx elastic_strain_yy strain_xx strain_yy'
    block = 'concrete'
  []
[]

[Materials]

  [water]
    type = ADGenericConstantMaterial
    block = 'water'
    prop_names = 'rho    k     cp      mu alpha_wall'
    prop_values = '955.7 0.6 ${fparse cp_water_multiplier * 4181} ${fparse 7.98e-4 * mu_multiplier} 30'
  []

  # Materials for thermo mechanics
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
    temperature = temperature
    thermal_expansion_coeff = 1e-5
    block = 'concrete'
  []
[]

[Executioner]
  type = Transient
  start_time = -1
  end_time = 200
  steady_state_tolerance = 1e-7
  steady_state_detection = true
  dt = 0.25
  solve_type = PJFNK
  automatic_scaling = true
  compute_scaling_once = false
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 500'
  line_search = none
  [TimeStepper]
    type = FunctionDT
    function = 'if(t<0,0.1,0.25)'
  []
[]

[Outputs]
  [out]
    type = Exodus
    elemental_as_nodal = true
  []
[]
