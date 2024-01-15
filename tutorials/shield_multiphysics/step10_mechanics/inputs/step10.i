[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../../step03_boundary_conditions/inputs/mesh_in.e'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [T]
    # Adds a Linear Lagrange variable by default
    block = 'concrete'
  []
[]

[Kernels]
  # [time_derivative]
  #   type = CoefTimeDerivative
  #   variable = T
  #   coef = '${fparse 2400 * 1170}'
  # []
  [diffusion_concrete]
    type = CoefDiffusion
    variable = T
    coef = 2.25
  []
[]

[Modules/TensorMechanics/Master]
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

[BCs]
  [from_reactor]
    type = NeumannBC
    variable = T
    boundary = inner_cavity
    # 100 kW reactor, 108 m2 cavity area
    value = '${fparse 1e5 / 108}'
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
  []
  [elastic_stress]
    type = ADComputeFiniteStrainElasticStress
    block = 'concrete'
  []
  [thermal_strain]
    type = ADComputeThermalExpansionEigenstrain
    stress_free_temperature = 300 # arbitrary value
    eigenstrain_name = eigenstrain
    temperature = T
    # inflated to get visible displacement
    # dont overdo it: 500K * alpha should be in the per-cent, max
    thermal_expansion_coeff = 2e-4 # arbitrary value
    block = 'concrete'
  []
[]

[Problem]
  type = FEProblem
  # No kernels on the water domain
  kernel_coverage_check = false
[]

[Executioner]
  # type = Transient
  type = Steady

  # start_time = -1
  # end_time = 200
  # steady_state_tolerance = 1e-7
  # steady_state_detection = true
  # dt = 0.25

  solve_type = PJFNK
  automatic_scaling = true
  # compute_scaling_once = false

  # petsc_options_iname = '-pc_type -pc_factor_shift_type'
  # petsc_options_value = 'lu NONZERO'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 500'
  line_search = none
  # [TimeStepper]
  #   type = FunctionDT
  #   function = 'if(t<0,0.1,0.25)'
  # []
[]

[Outputs]
  exodus = true # Output Exodus format

  [displaced]
    type = Exodus
    use_displaced = true
  []
[]
