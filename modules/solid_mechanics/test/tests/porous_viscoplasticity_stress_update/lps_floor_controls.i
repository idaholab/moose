# Generic scalar-floor control regression. Hydrostatic compression supplies nonzero unconstrained
# creep driving while the active porosity floor admits only numerical-noise-level controller motion.
# The floor must remain stationary, the local controller must not refine solely because of blocked
# motion, and the global material timestep recommendation must remain effectively unrestricted.

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  coord_type = RZ
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
  [pressure]
    type = ParsedFunction
    expression = '5e7'
  []
[]

[BCs]
  [axis]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [bottom]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [Pressure]
    [hydrostatic]
      boundary = 'right top'
      function = pressure
      use_automatic_differentiation = true
    []
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
    prop_values = 1e-8
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
    use_substepping = NONE
  []
[]

[Postprocessors]
  [porosity]
    type = ADElementAverageMaterialProperty
    mat_prop = porosity
    execute_on = 'TIMESTEP_END INITIAL'
  []
  [admitted_controller_rate]
    type = ADElementAverageMaterialProperty
    mat_prop = substep_control_effective_viscoplasticity_rate
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
    message = 'Generic scalar porosity floor was violated.'
  []
  [check_admitted_controller_rate]
    type = Terminator
    expression = 'abs(admitted_controller_rate) > 1e-12'
    execute_on = FINAL
    fail_mode = HARD
    error_level = ERROR
    message = 'Stationary generic scalar floor retained a non-negligible admitted substep-control rate.'
  []
  [check_unrestricted_material_timestep]
    type = Terminator
    expression = 'material_time_step_limit != material_time_step_limit | material_time_step_limit < 1e9'
    execute_on = FINAL
    fail_mode = HARD
    error_level = ERROR
    message = 'Blocked generic floor motion materially restricted the global material timestep.'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.1
  end_time = 0.2
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
