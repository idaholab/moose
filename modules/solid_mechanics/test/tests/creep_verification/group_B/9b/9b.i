# Units: N, mm, hour

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

[Functions]
  [coefficientFunction]
    type = ParsedFunction
    expression = 'if(t <= 0, 0, 1.5625e-14)'
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

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = SMALL
    incremental = true
    planar_formulation = WEAK_PLANE_STRESS
    use_automatic_differentiation = true

    generate_output = 'stress_xx stress_yy stress_xy stress_zz strain_xx strain_yy strain_xy creep_strain_xx creep_strain_yy creep_strain_zz creep_strain_xy'
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

  # Line BC: prescribed x displacement u1 = 0.1 mm
  [prescribed_x_on_BC]
    type = ADDirichletBC
    variable = disp_x
    boundary = right
    value = 0.1
  []

  # Line CD: prescribed y displacement u2 = 0.1 mm
  [prescribed_y_on_CD]
    type = ADDirichletBC
    variable = disp_y
    boundary = top
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
    inelastic_models = 'creep'
    perform_finite_strain_rotations = false
  []

  [creep]
    type = ADPowerLawCreepStressUpdate

    # A*m = 3.125e-14*0.5
    coefficient = coefficientFunction

    n_exponent = 5.0

    # m-1 = 0.5-1
    m_exponent = -0.5

    activation_energy = 0.0
    start_time = -1

    effective_inelastic_strain_name = effective_creep_strain

    # Constitutive substepping
    max_inelastic_increment = 1.0e-4
    use_substepping = INCREMENT_BASED
    maximum_number_substeps = 1000

    absolute_tolerance = 1.0e-11
    relative_tolerance = 1.0e-8
  []

[]

[Postprocessors]
  # Material-recommended time-step limit
  [material_dt]
    type = MaterialTimeStepPostprocessor
  []

  [avg_eff_creep]
    type = ElementAverageValue
    variable = effective_creep_strain
  []

  # Displacement checks

  [right_disp_x]
    type = SideAverageValue
    variable = disp_x
    boundary = right
  []

  # Stress checks
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
  start_time = -1.0
  end_time = 1000.0

  dtmin = 1.0e-12

  nl_max_its = 50
  l_max_its = 200

  nl_rel_tol = 1.0e-9
  nl_abs_tol = 1.0e-11
  l_tol = 1.0e-10


  automatic_scaling = true

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  # Fine time resolution near t = 0 because the
  # time-hardening creep rate varies as t^(-0.5).
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
