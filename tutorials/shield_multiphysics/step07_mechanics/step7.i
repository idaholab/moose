[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../step03_boundary_conditions/mesh_in.e'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [T]
    # Adds a Linear Lagrange variable by default
    block = 'concrete_hd concrete Al'
  []
[]

[Kernels]
  [diffusion_concrete]
    type = ADHeatConduction
    variable = T
  []

  [gravity]
    type = Gravity
    variable = 'disp_z'
    value = '-9.81'
    block = 'concrete_hd concrete Al'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    # This block adds all of the proper Kernels, strain calculators, and Variables
    # for Solid Mechanics equations in the correct coordinate system (autodetected)
    add_variables = true
    strain = FINITE
    eigenstrain_names = eigenstrain
    use_automatic_differentiation = true
    generate_output = 'vonmises_stress elastic_strain_xx elastic_strain_yy strain_xx strain_yy'
    block = 'concrete_hd concrete Al'
  []
[]

[BCs]
  [from_reactor]
    type = NeumannBC
    variable = T
    boundary = inner_cavity_solid
    # 5 MW reactor, only 50 kW removed from radiation, 144 m2 cavity area
    value = '${fparse 5e4 / 144}'
  []
  [air_convection]
    type = ADConvectiveHeatFluxBC
    variable = T
    boundary = 'air_boundary'
    T_infinity = 300.0
    # The heat transfer coefficient should be obtained from a correlation
    heat_transfer_coefficient = 10
  []
  [ground]
    type = DirichletBC
    variable = T
    value = 300
    boundary = 'ground'
  []
  [water_convection]
    type = ADConvectiveHeatFluxBC
    variable = T
    boundary = 'water_boundary_inwards'
    T_infinity = 300.0
    # The heat transfer coefficient should be obtained from a correlation
    heat_transfer_coefficient = 600
  []

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

[Materials]
  [concrete_hd]
    type = ADHeatConductionMaterial
    block = concrete_hd
    temp = 'T'
    # we specify a function of time, temperature is passed as the time argument
    # in the material
    thermal_conductivity_temperature_function = '5.0 + 0.001 * t'
  []
  [concrete]
    type = ADHeatConductionMaterial
    block = concrete
    temp = 'T'
    thermal_conductivity_temperature_function = '2.25 + 0.001 * t'
  []
  [Al]
    type = ADHeatConductionMaterial
    block = Al
    temp = T
    thermal_conductivity_temperature_function = '175'
  []

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

  # NOTE: This handles thermal expansion by coupling to the displacements
  [density_concrete_hd]
    type = Density
    block = 'concrete_hd'
    density = '3524' # kg / m3
  []
  [density_concrete]
    type = Density
    block = 'concrete'
    density = '2403' # kg / m3
  []
  [density_Al]
    type = Density
    block = 'Al'
    density = '2270' # kg / m3
  []
[]

[Problem]
  type = FEProblem
  # No kernels on the water domain
  kernel_coverage_check = false
  # No materials defined on the water domain
  material_coverage_check = false
[]

[Executioner]
  type = Steady

  solve_type = NEWTON
  automatic_scaling = true

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 500'
  line_search = none
[]

[Outputs]
  exodus = true
[]
