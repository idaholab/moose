# Generic mixed-loading floor regression. Volumetric collapse is blocked at minimum_porosity while
# the deviatoric part of the same LPS law remains active. The admitted-motion controller must retain
# a finite material timestep recommendation from the surviving deviatoric creep.

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
  [disp_x_function]
    type = ParsedFunction
    expression = '(-1e-4*x + 1e-4*y) * t / 0.1'
  []
  [disp_y_function]
    type = ParsedFunction
    expression = '-1e-4*y * t / 0.1'
  []
[]

[BCs]
  [disp_x]
    type = ADFunctionDirichletBC
    variable = disp_x
    boundary = 'left right top bottom'
    function = disp_x_function
  []
  [disp_y]
    type = ADFunctionDirichletBC
    variable = disp_y
    boundary = 'left right top bottom'
    function = disp_y_function
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
    prop_names = creep_coefficient
    prop_values = 1e-10
  []
  [stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = lps
  []
  [porosity]
    type = ADPorosityFromStrain
    initial_porosity = 0.05
    inelastic_strain = combined_inelastic_strain
  []
  [lps]
    type = ADPorousViscoplasticityStressUpdate
    coefficient = creep_coefficient
    power = 1
    initial_porosity = 0.05
    minimum_porosity = 0.05
    max_inelastic_increment = 1e-2
  []
[]

[Postprocessors]
  [porosity]
    type = ADElementAverageMaterialProperty
    mat_prop = porosity
    execute_on = 'TIMESTEP_END INITIAL'
  []
  [effective_viscoplasticity]
    type = ADElementAverageMaterialProperty
    mat_prop = effective_viscoplasticity
    execute_on = 'TIMESTEP_END INITIAL'
  []
  [material_time_step_limit]
    type = MaterialTimeStepPostprocessor
    maximum_value = 1e30
    execute_on = TIMESTEP_END
  []
[]

[UserObjects]
  [check_floor]
    type = Terminator
    expression = 'abs(porosity - 0.05) > 1e-10'
    execute_on = FINAL
    fail_mode = HARD
    error_level = ERROR
    message = 'Mixed generic loading violated the scalar porosity floor.'
  []
  [check_admitted_creep_remains_active]
    type = Terminator
    expression = 'effective_viscoplasticity <= 1e-10'
    execute_on = FINAL
    fail_mode = HARD
    error_level = ERROR
    message = 'The generic scalar-floor active set incorrectly suppressed all admitted creep under mixed loading.'
  []
  [check_finite_material_timestep]
    type = Terminator
    expression = 'material_time_step_limit != material_time_step_limit | material_time_step_limit <= 0 | material_time_step_limit >= 1e29'
    execute_on = FINAL
    fail_mode = HARD
    error_level = ERROR
    message = 'Admitted deviatoric flow at the generic floor did not provide a finite timestep limit.'
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
  csv = false
[]
