# Pure-deviatoric integrated regression for simultaneous generic porous-LPS creep mechanisms.

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmax = 1e-3
  ymax = 1e-3
[]

[Physics/SolidMechanics/QuasiStatic/All]
  strain = FINITE
  add_variables = true
  use_automatic_differentiation = true
[]

[Functions]
  [shear]
    type = ParsedFunction
    expression = '1e-5 * t / 0.1'
  []
[]

[BCs]
  [bottom_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = bottom
    value = 0
  []
  [bottom_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [top_x]
    type = ADFunctionDirichletBC
    variable = disp_x
    boundary = top
    function = shear
  []
  [top_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = top
    value = 0
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.3
  []
  [constants]
    type = ADGenericConstantMaterial
    prop_names = 'coef_linear coef_cubic coef_45'
    prop_values = '1e-10 1e-20 3e-28'
  []
  [stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = lps
  []
  [porosity]
    type = ADPorosityFromStrain
    initial_porosity = 0.1
    inelastic_strain = combined_inelastic_strain
  []
  [lps]
    type = ADPorousViscoplasticityStressUpdate
    coefficient = 'coef_linear coef_cubic coef_45'
    power = '1 3 4.5'
    porosity_name = porosity
    max_inelastic_increment = 1e-2
    relative_tolerance = 1e-12
    absolute_tolerance = 1e-12
  []
[]

[Postprocessors]
  [porosity]
    type = ADElementAverageMaterialProperty
    mat_prop = porosity
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [effective_viscoplasticity]
    type = ADElementAverageMaterialProperty
    mat_prop = effective_viscoplasticity
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [gauge_legacy]
    type = ADElementAverageMaterialProperty
    mat_prop = gauge_stress
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [gauge_0]
    type = ADElementAverageMaterialProperty
    mat_prop = gauge_stress_0
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [gauge_1]
    type = ADElementAverageMaterialProperty
    mat_prop = gauge_stress_1
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [gauge_2]
    type = ADElementAverageMaterialProperty
    mat_prop = gauge_stress_2
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[UserObjects]
  [check_deviatoric_flow]
    type = Terminator
    expression = 'effective_viscoplasticity <= 1e-10'
    execute_on = FINAL
    error_level = ERROR
    message = 'Pure-shear loading did not accumulate generic porous-LPS inelastic strain.'
  []
  [check_porosity]
    type = Terminator
    expression = 'abs(porosity - 0.1) > 1e-6'
    execute_on = FINAL
    error_level = ERROR
    message = 'Finite-strain pure-shear generic porous-LPS flow produced excessive net porosity change.'
  []
  [check_legacy_gauge]
    type = Terminator
    expression = 'abs(gauge_legacy - gauge_0) / (abs(gauge_0) + 1) > 1e-12'
    execute_on = FINAL
    error_level = ERROR
    message = 'The legacy gauge_stress output did not retain the first active creep mechanism.'
  []
  [check_gauges]
    type = Terminator
    expression = 'gauge_0 <= 0 | gauge_1 <= 0 | gauge_2 <= 0'
    execute_on = FINAL
    error_level = ERROR
    message = 'One or more generic porous-LPS gauge stresses were nonpositive under pure shear.'
  []
  [check_distinct_n3_gauge]
    type = Terminator
    expression = 'abs(gauge_1 - gauge_0) / (abs(gauge_0) + 1) < 1e-4'
    execute_on = FINAL
    error_level = ERROR
    message = 'The n=1 and n=3 LPS mechanisms unexpectedly produced the same gauge stress.'
  []
  [check_distinct_n45_gauge]
    type = Terminator
    expression = 'abs(gauge_2 - gauge_0) / (abs(gauge_0) + 1) < 1e-4'
    execute_on = FINAL
    error_level = ERROR
    message = 'The n=1 and n=4.5 LPS mechanisms unexpectedly produced the same gauge stress.'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.01
  end_time = 0.1
  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-10
  l_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  print_linear_residuals = false
[]
