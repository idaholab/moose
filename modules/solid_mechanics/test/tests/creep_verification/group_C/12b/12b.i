[GlobalParams]
  displacements = 'disp_x disp_y'
  out_of_plane_strain = strain_zz
[]

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1

    xmin = 0.0
    xmax = 100.0
    ymin = 0.0
    ymax = 100.0

    elem_type = QUAD8
  []

  # Midpoint of line AD
  [mid_AD]
    type = ExtraNodesetGenerator
    input = square
    new_boundary = mid_AD
    coord = '0.0 50.0 0.0'
  []
[]

[Functions]
  [coefficientFunctionPrimary]
    type = ParsedFunction
    expression = 'if(t <= 0, 0, 5.0e-15)'
  []
  [coefficientFunctionSecondary]
    type = ParsedFunction
    expression = 'if(t <= 0, 0, 1.0e-16)'
  []
[]

[Variables]
  # Required for WEAK_PLANE_STRESS
  [strain_zz]
    order = SECOND
    family = LAGRANGE
  []
[]

[AuxVariables]
  # Combined primary + secondary creep strain
  [combined_creep_xx]
    order = CONSTANT
    family = MONOMIAL
  []

  [combined_creep_yy]
    order = CONSTANT
    family = MONOMIAL
  []

  [combined_creep_zz]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [combined_creep_xx]
    type = ADRankTwoAux
    variable = combined_creep_xx
    rank_two_tensor = combined_inelastic_strain
    index_i = 0
    index_j = 0
    execute_on = 'initial timestep_end'
  []

  [combined_creep_yy]
    type = ADRankTwoAux
    variable = combined_creep_yy
    rank_two_tensor = combined_inelastic_strain
    index_i = 1
    index_j = 1
    execute_on = 'initial timestep_end'
  []

  [combined_creep_zz]
    type = ADRankTwoAux
    variable = combined_creep_zz
    rank_two_tensor = combined_inelastic_strain
    index_i = 2
    index_j = 2
    execute_on = 'initial timestep_end'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = SMALL
    incremental = true
    planar_formulation = WEAK_PLANE_STRESS
    use_automatic_differentiation = true

    generate_output = '
      stress_xx
      stress_yy
      stress_zz
      stress_xy

      strain_xx
      strain_yy
      strain_xy
    '
  []
[]

[BCs]
  # Line AD: ux = 0
  [fix_AD_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []

  # Midpoint of AD: uy = 0
  [fix_mid_AD_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = mid_AD
    value = 0.0
  []

  # Line BC: ux = 0.1 mm
  [prescribe_BC_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = right
    value = 0.1
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 2.0e5
    poissons_ratio = 0.3
  []

  [stress]
    type = ADComputeMultipleInelasticStress

    inelastic_models = '
      secondary_creep
      primary_creep
    '

    # Combined creep strain = secondary + primary
    combined_inelastic_strain_weights = '1.0 1.0'

    perform_finite_strain_rotations = false

    max_iterations = 50
    absolute_tolerance = 1e-12
    relative_tolerance = 1e-10
  []

  # Secondary creep:
  #
  # epsilon_c1 = A1 sigma^n1 t
  #
  # A1 = 1e-16
  # n1 = 5
  [secondary_creep]
    type = ADPowerLawCreepStressUpdate

    base_name = secondary

    coefficient = coefficientFunctionSecondary
    n_exponent = 5.0
    m_exponent = 0.0

    activation_energy = 0.0
    start_time = -1

    max_inelastic_increment = 1e-4
 []

  # Primary creep:
  #
  # epsilon_c2 = A2 sigma^n2 t^m
  #
  # A2 = 1e-14
  # n2 = 5
  # m  = 0.5
  #
  # Rate-law conversion:
  #
  # coefficient = A2*m = 5e-15
  # m_exponent  = m-1 = -0.5
  [primary_creep]
    type = ADPowerLawCreepStressUpdate

    base_name = primary

    coefficient = coefficientFunctionPrimary
    n_exponent = 5.0
    m_exponent = -0.5

    activation_energy = 0.0
    start_time = -1

    max_inelastic_increment = 1e-4
  []
[]

[Postprocessors]
  [material_dt]
    type = MaterialTimeStepPostprocessor
  []

  # Prescribed displacement check
  [right_disp_x]
    type = SideAverageValue
    variable = disp_x
    boundary = right
  []

  [avg_stress_xx]
    type = ElementAverageValue
    variable = stress_xx
  []

  [avg_stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  []

  [avg_stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  []

  [avg_stress_xy]
    type = ElementAverageValue
    variable = stress_xy
  []

  # Combined primary + secondary creep strain
  [avg_creep_xx]
    type = ElementAverageValue
    variable = combined_creep_xx
  []

  [avg_creep_yy]
    type = ElementAverageValue
    variable = combined_creep_yy
  []

  [avg_creep_zz]
    type = ElementAverageValue
    variable = combined_creep_zz
  []

  [avg_strain_xx]
    type = ElementAverageValue
    variable = strain_xx
  []

  [avg_strain_yy]
    type = ElementAverageValue
    variable = strain_yy
  []

  [avg_strain_zz]
    type = ElementAverageValue
    variable = strain_zz
  []

[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  # Time unit: hour
  start_time = -1
  end_time = 1000.0

  nl_max_its = 50
  l_max_its = 100

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-12

  automatic_scaling = true

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  # The converted rate contains t^(-0.5); logarithmic stepping
  # resolves the rapid initial stress relaxation.
  [TimeStepper]
    type = IterationAdaptiveDT

    # Small initial step because the primary-creep rate
    # is proportional to t^(-0.5)
    dt = 1

    growth_factor = 1.25
    cutback_factor = 0.25

    optimal_iterations = 8
    iteration_window = 2

    timestep_limiting_postprocessor = material_dt

    time_t = '-1 0'
    time_dt = '1 1e-6'
  []
[]

[Outputs]
  exodus = true
  show = "avg_stress_xx avg_stress_yy avg_stress_zz"
  execute_on = 'timestep_end'
  [full]
    type = CSV
  []
  [out]
    type = CSV
    sync_times = '0 0.1 1 5 10 50 100 500 1000'
    sync_only = true
  []
[]
