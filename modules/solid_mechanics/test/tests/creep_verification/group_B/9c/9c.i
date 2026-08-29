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
  # Both applied stresses follow the same history.
  #
  # The benchmark specifies 200 N/mm^2 at exactly t = 100 h
  # and 250 N/mm^2 for t > 100 h.
  [applied_stress]
    type = ParsedFunction
    expression = 'if(t <= 100.0, 200.0, 250.0)'
  []

  # Time-hardening axial creep-strain reference
  #
  # Before the load increase:
  #
  # After the load increase:
  [creep_xx_time_hardening_reference_function]
    type = ParsedFunction
    expression = 'if(t <= 100.0, 0.005*sqrt(t), 0.05 + 0.0152587890625*(sqrt(t)-10.0))'
  []

  # Strain-hardening axial creep-strain reference
  #
  # At the load change, the accumulated creep strain is
  # transferred to the equivalent point on the 250-MPa curve.
  #
  # Equivalent time at 250 MPa:
  #   t_eq = 10.73741824 h
  #
  # For t > 100:
  #
  #   epsilon_xx^c =
  #     0.0152587890625*sqrt(t-89.26258176)
  [creep_xx_strain_hardening_reference_function]
    type = ParsedFunction
    expression = 'if(t <= 100.0, 0.005*sqrt(t), 0.0152587890625*sqrt(t-89.26258176))'
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

  # Line BC: time-dependent tensile stress sigma_1
  [traction_x_on_BC]
    type = ADFunctionNeumannBC
    variable = disp_x
    boundary = right
    function = applied_stress
  []

  # Line CD: time-dependent tensile stress sigma_2
  [traction_y_on_CD]
    type = ADFunctionNeumannBC
    variable = disp_y
    boundary = top
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

    max_inelastic_increment = 1.0e-3
    use_substepping = INCREMENT_BASED
    maximum_number_substeps = 1000

    absolute_tolerance = 1.0e-11
    relative_tolerance = 1.0e-8
  []

[]

[Postprocessors]
  # Applied traction diagnostic
  [applied_stress_value]
    type = FunctionValuePostprocessor
    function = applied_stress
    point = '0 0 0'
  []

  # MOOSE-computed time-hardening creep response
  [creep_xx_time_hardening]
    type = ElementAverageValue
    variable = creep_strain_xx
    execute_on = TIMESTEP_END
  []

  [creep_yy_time_hardening]
    type = ElementAverageValue
    variable = creep_strain_yy
  []

  [creep_zz_time_hardening]
    type = ElementAverageValue
    variable = creep_strain_zz
    execute_on = TIMESTEP_END
  []

  # Analytical time-hardening reference
  [creep_xx_time_hardening_reference]
    type = FunctionValuePostprocessor
    function = creep_xx_time_hardening_reference_function
    point = '0 0 0'
    execute_on = TIMESTEP_END
  []

  # Analytical strain-hardening reference
  [creep_xx_strain_hardening_reference]
    type = FunctionValuePostprocessor
    function = creep_xx_strain_hardening_reference_function
    point = '0 0 0'
    execute_on = TIMESTEP_END
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
  l_max_its = 200

  nl_rel_tol = 1.0e-9
  nl_abs_tol = 1.0e-11
  l_tol = 1.0e-10

  automatic_scaling = true

  dtmin = 1.0e-12

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  [TimeStepper]
    type = LogConstantDT
    first_dt = 1.0e-4
    log_dt = 0.01
    cutback_factor_at_failure = 0.25
  []
[]

[Outputs]
  exodus = true
  show = 'avg_creep_xx avg_creep_yy avg_creep_zz'
  execute_on = 'timestep_end'
  [full]
    type = CSV
  []
  [out]
    type = CSV
    sync_times = '0 1 10 50 100 110 120 150 200'
    sync_only = true
  []
[]
