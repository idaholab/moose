# creep_plate_primary_step_load.i
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

  # Midpoint of line AD
  [mid_AD]
    type = ExtraNodesetGenerator
    input = square
    new_boundary = mid_AD
    coord = '0.0 50.0 0.0'
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
  # Applied traction:
  # Use t < 100 so that the load changes to 250 MPa at the
  # solution point t = 100 h.
  [applied_stress]
    type = ParsedFunction
    expression = 'if(t < 100.0, 200.0, 250.0)'
  []

  # Time-hardening analytical response
  #
  # Before load change:
  #   eps_c = A*(200)^5*sqrt(t) = 0.01*sqrt(t)
  #
  # After load change:
  #   eps_c = eps_c(100)
  #         + A*(250)^5*(sqrt(t)-sqrt(100))
  [creep_xx_time_hardening_reference]
    type = ParsedFunction
    expression = 'if(t <= 100.0,
                     0.01*sqrt(t),
                     0.1 + 0.030517578125*(sqrt(t)-10.0))'
  []

  # Strain-hardening analytical response
  #
  # The creep strain accumulated at 200 MPa is transferred to
  # the equivalent position on the 250 MPa creep curve.
  [creep_xx_strain_hardening_function]
    type = ParsedFunction
    expression = 'if(t <= 100.0,
                     0.01*sqrt(t),
                     sqrt(0.01 +
                          0.0009313225746154785*(t-100.0)))'
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

  # Midpoint of AD: uy = 0
  [fix_mid_AD_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = mid_AD
    value = 0.0
  []

  # Line BC: time-dependent tensile traction
  [tensile_traction_BC]
    type = ADFunctionNeumannBC
    variable = disp_x
    boundary = right
    function = applied_stress
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
  []

  [creep]
    type = ADPowerLawCreepStressUpdate

    # A*m = 3.125e-14*0.5
    coefficient = 1.5625e-14

    n_exponent = 5.0

    # m-1 = 0.5-1
    m_exponent = -0.5

    activation_energy = 0.0
    start_time = 0.0

    effective_inelastic_strain_name = effective_creep_strain

    max_inelastic_increment = 0.01
    use_substepping = INCREMENT_BASED
    maximum_number_substeps = 500

    absolute_tolerance = 1e-12
    relative_tolerance = 1e-10
  []

[]

[Postprocessors]
  # Applied traction diagnostic
  [applied_stress_value]
    type = FunctionValuePostprocessor
    function = applied_stress
    execute_on = 'INITIAL TIMESTEP_END'
  []

  # Requested creep_xx outputs

  # MOOSE-computed time-hardening result
  [creep_xx_time_hardening]
    type = ElementAverageValue
    variable = creep_strain_xx
    execute_on = 'INITIAL TIMESTEP_END'
  []

  # Analytical strain-hardening result
  [creep_xx_strain_hardening]
    type = FunctionValuePostprocessor
    function = creep_xx_strain_hardening_function
    execute_on = 'INITIAL TIMESTEP_END'
  []

  # Optional analytical check of the MOOSE time-hardening result
  [creep_xx_time_hardening_exact]
    type = FunctionValuePostprocessor
    function = creep_xx_time_hardening_reference
    execute_on = 'INITIAL TIMESTEP_END'
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
  start_time = 0.0
  end_time = 200.0


  nl_max_its = 50
  l_max_its = 100

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-12

  automatic_scaling = true

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  [TimeStepper]
    type = LogConstantDT
    first_dt = 1e-6
    log_dt = 0.01
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
    sync_times = '0 0.1 10 50 100 110 120 150 200'
    sync_only = true
  []
[]
