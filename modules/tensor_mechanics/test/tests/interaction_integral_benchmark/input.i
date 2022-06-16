# Uses InteractionIntegralBenchmarkBC to test the mixed-mode stress intensity
# factor capability. InteractionIntegralBenchmarkBC applies a displacement
# field for which KI = KII = KIII = 1.0. Using the option 2d = true gives a
# q field that is constant along the tangent and returns Ki = 1.0 for all i.
# To get the correct value for all nodes with 2d = false, the mesh around the
# crack tip must be refined and the q-function radii must be reduced by at
# least two orders of magnitude.

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[Mesh]
  file = 360degree_model.e
  displacements = 'disp_x disp_y disp_z'
[]

[Problem]
  type = FEProblem
[]

[Functions]
  [./kifunc]
    type = PiecewiseLinear
    x = '0.0 1.0 2.0'
    y = '0.0 1.0 2.0'
  [../]
[]

[DomainIntegral]
  integrals = 'JIntegral InteractionIntegralKI InteractionIntegralKII InteractionIntegralKIII'
  boundary = 1001
  crack_direction_method = CrackDirectionVector
  crack_direction_vector = '1 0 0'
  radius_inner = '0.5 1.0 1.5 2.0'
  radius_outer = '1.0 1.5 2.0 2.5'
  youngs_modulus = 30000
  poissons_ratio = 0.3
  block = 1
  2d = true
  axis_2d = 2
  equivalent_k = True
  incremental = true
[]

[AuxVariables]
  [./dq_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./dq_y]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./dq_z]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Modules/TensorMechanics/Master]
  [./master]
    strain = FINITE
    add_variables = true
    incremental = true
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress'
  [../]
[]

[BCs]
  [./all_x]
    type = InteractionIntegralBenchmarkBC
    variable = disp_x
    component = x
    boundary = 1
    KI_function = kifunc
    KII_function = 1.0
    KIII_function = 1.0
    youngs_modulus = 30000
    poissons_ratio = 0.3
    crack_front_definition = crackFrontDefinition
    crack_front_point_index = 0
  [../]
  [./all_y]
    type = InteractionIntegralBenchmarkBC
    variable = disp_y
    component = y
    boundary = 1
    KI_function = kifunc
    KII_function = 1.0
    KIII_function = 1.0
    youngs_modulus = 30000
    poissons_ratio = 0.3
    crack_front_definition = crackFrontDefinition
    crack_front_point_index = 0
  [../]
  [./all_z]
    type = InteractionIntegralBenchmarkBC
    variable = disp_z
    component = z
    boundary = 1
    KI_function = kifunc
    KII_function = 1.0
    KIII_function = 1.0
    youngs_modulus = 30000
    poissons_ratio = 0.3
    crack_front_definition = crackFrontDefinition
    crack_front_point_index = 0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 30000
    poissons_ratio = 0.3
  [../]
  [./elastic_stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Executioner]
  type = Transient

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 50
  nl_max_its = 20
  nl_abs_tol = 1e-3
  l_tol = 1e-2

  start_time = 0.0
  dt = 1

  end_time = 2
  num_steps = 2
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]

  [./nl_its]
    type = NumNonlinearIterations
  [../]

  [./lin_its]
    type = NumLinearIterations
  [../]

[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = 360degree_model_out
  csv = true
[]
