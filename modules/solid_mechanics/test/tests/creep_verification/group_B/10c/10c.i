# Units: N, mm, hour
#
# Applied stress:
#   sigma_xx = 200 N/mm^2 for t <= 100 h
#   sigma_xx = 250 N/mm^2 for t >  100 h
#
# NAFEMS primary-creep relation:
#   epsilon_c = A * sigma^n * t^m
#
#   A = 3.125e-14
#   n = 5
#   m = 0.5
#
# Equivalent MOOSE time-hardening rate law:
#   epsilon_dot_c = A*m * sigma^n * t^(m-1)
#
#   coefficient = 1.5625e-14
#   m_exponent  = -0.5

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
[]

[Variables]
  # Required for WEAK_PLANE_STRESS.
  [strain_zz]
    order = SECOND
    family = LAGRANGE
  []
[]

[AuxVariables]
  [effective_creep_strain]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [effective_creep_strain]
    type = ADMaterialRealAux
    variable = effective_creep_strain
    property = effective_creep_strain
  []
[]

[Functions]
  # sigma_xx:
  # 200 N/mm2 through t = 100 h
  # 250 N/mm2 for t > 100 h
  [sigma_x]
    type = ParsedFunction
    expression = 'if(t <= 100.0, 200.0, 250.0)'
  []

  # sigma_yy:
  # -200 N/mm2 through t = 100 h
  # -250 N/mm2 for t > 100 h
  [sigma_y]
    type = ParsedFunction
    expression = 'if(t <= 100.0, -200.0, -250.0)'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = SMALL
    incremental = true

    planar_formulation = WEAK_PLANE_STRESS
    use_automatic_differentiation = true

    generate_output = 'stress_xx
                       stress_yy
                       stress_xy
                       stress_zz
                       vonmises_stress
                       strain_xx
                       strain_yy
                       strain_xy
                       creep_strain_xx
                       creep_strain_yy
                       creep_strain_zz
                       creep_strain_xy'
  []
[]

[BCs]
  # Line AD: prescribed zero x-displacement
  [fix_x_on_AD]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []

  # Line AB: zero y displacement
  [fix_y_on_AB]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []

  # Line BC: tensile sigma_xx
  [apply_sigma_x]
    type = ADFunctionNeumannBC
    variable = disp_x
    boundary = right
    function = sigma_x
  []

  # Line CD: compressive sigma_yy
  [apply_sigma_y]
    type = ADFunctionNeumannBC
    variable = disp_y
    boundary = top
    function = sigma_y
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
    inelastic_models = 'creep'

    perform_finite_strain_rotations = false
  []

  [creep]
    type = ADPowerLawCreepStressUpdate

    # Benchmark accumulated creep law:
    #
    # epsilon_eff_c = A sigma_eff^n t^m
    #
    # A = 3.125e-14
    # n = 5
    # m = 0.5
    #
    # ADPowerLawCreepStressUpdate uses the rate:
    #
    # dot(epsilon_eff_c) = C sigma_eff^n t^q
    #
    # C = A*m   = 1.5625e-14
    # q = m - 1 = -0.5

    coefficient = 1.5625e-14

    n_exponent = 5.0

    # m-1 = 0.5-1
    m_exponent = -0.5

    activation_energy = 0.0
    start_time = 0.0

    effective_inelastic_strain_name = effective_creep_strain

    max_inelastic_increment = 1e-3
  []
[]

[Postprocessors]
  [material_dt]
    type = MaterialTimeStepPostprocessor
  []

  [current_dt]
    type = TimestepSize
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

  [avg_creep_xx]
    type = ElementAverageValue
    variable = creep_strain_xx
  []

  [avg_creep_zz]
    type = ElementAverageValue
    variable = creep_strain_zz
  []

  [avg_creep_xy]
    type = ElementAverageValue
    variable = creep_strain_xy
  []

  [avg_effective_creep]
    type = ElementAverageValue
    variable = effective_creep_strain
  []

  [avg_creep_yy]
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
  start_time = 0.0
  end_time = 200.0

  dtmin = 1e-12
  dtmax = 1.0

  nl_max_its = 50
  l_max_its = 100

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-12

  automatic_scaling = true

  [TimeStepper]
    type = IterationAdaptiveDT

    # Small initial increment because the time-hardening
    # creep rate is proportional to t^(-0.5)
    dt = 1e-6

    growth_factor = 2.0
    cutback_factor = 0.5

    optimal_iterations = 8
    iteration_window = 2

    timestep_limiting_postprocessor = material_dt
  []
[]

[Outputs]
  exodus = true
  show = 'avg_creep_xx avg_creep_yy avg_creep_zz'
  execute_on = 'initial timestep_end'
  [full]
    type = CSV
  []
  [out]
    type = CSV
    sync_times = '0 1 10 50 100 110 120 150 200'
    sync_only = true
  []
[]
