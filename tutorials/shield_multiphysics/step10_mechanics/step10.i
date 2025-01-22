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
    block = 'concrete concrete_and_Al'
  []
[]

[Kernels]
  [diffusion_concrete]
    type = CoefDiffusion
    variable = T
    coef = 2.25
  []

  [gravity]
    type = Gravity
    variable = 'disp_z'
    value = '-9.81'
    block = 'concrete concrete_and_Al'
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
    block = 'concrete concrete_and_Al'
  []
[]

[BCs]
  [from_reactor]
    type = NeumannBC
    variable = T
    boundary = inner_cavity
    # 5 MW reactor, 50kW removed by radiation, 108 m2 cavity area
    value = '${fparse 5e4 / 108}'
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
    heat_transfer_coefficient = 30
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
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 200e9 # (Pa) arbitrary value
    poissons_ratio = .3 # arbitrary value
    block = 'concrete concrete_and_Al'
  []
  [elastic_stress]
    type = ADComputeFiniteStrainElasticStress
    block = 'concrete concrete_and_Al'
  []
  [thermal_strain]
    type = ADComputeThermalExpansionEigenstrain
    stress_free_temperature = 300 # arbitrary value
    eigenstrain_name = eigenstrain
    temperature = T
    # inflated to get visible displacement
    # don't overdo it: 500K * alpha should be in the per-cent, max
    thermal_expansion_coeff = 2e-4 # arbitrary value
    block = 'concrete concrete_and_Al'
  []
  # NOTE: This handles thermal expansion by coupling to the displacements
  [concrete_density]
    type = Density
    density = '2400'
    block = 'concrete concrete_and_Al'
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

  solve_type = PJFNK
  automatic_scaling = true

  # petsc_options_iname = '-pc_type -pc_factor_shift_type'
  # petsc_options_value = 'lu NONZERO'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 500'
  line_search = none
[]

[Outputs]
  exodus = true # Output Exodus format

  # Outputs the displaced mesh
  [displaced]
    type = Exodus
    use_displaced = true
  []
[]
