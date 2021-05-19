starting_point = 2e-1
offset = 1e-2

[GlobalParams]
  displacements = 'disp_x disp_y'
  # scaling = 1.0
[]

[Mesh]
  file = long-bottom-block-1elem-blocks.e
  uniform_refine = 0
  patch_update_strategy = always
[]

[Variables]
  [disp_x]
    block = '1 2'
  []
  [disp_y]
    block = '1 2'
  []
  [normal_lm]
    block = 3
    use_dual = true
  []
  # [gap_variable]
  #   block = 3
  #   use_dual = false
  # []
  # [vel_variable]s
  #   block = 3
  #   use_dual = false
  # []
  [tangential_lm]
    block = 3
    use_dual = true
  []
[]

[ICs]
  [disp_y]
    block = 2
    variable = disp_y
    value = '${fparse starting_point + offset}'
    type = ConstantIC
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    generate_output = 'stress_xx stress_yy'
    block = '1 2'
  []
[]

[Materials]
  [elasticity_2]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1e3
    poissons_ratio = 0.3
  []
  [elasticity_1]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  []
[]
# [Problem]
#   error_on_jacobian_nonzero_reallocation = true
# []
[Constraints]
  [weighted_gap_lm]
    type = ComputeFrictionalForceLMMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    variable = normal_lm
    disp_x = disp_x
    disp_y = disp_y
    use_displaced_mesh = true
    friction_lm = tangential_lm
    mu = 0.4
    c_t = 1.0e1 # TODO compaare
    c = 1.0e3
  []
  [normal_x]
    type = NormalMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    variable = normal_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
  []
  [normal_y]
    type = NormalMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    variable = normal_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
  []
  [tangential_x]
    type = TangentialMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    variable = tangential_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
  []
  [tangential_y]
    type = TangentialMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    variable = tangential_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
  []
[]

[BCs]
  [botx]
    type = DirichletBC
    variable = disp_x
    boundary = '40'
    value = 0.0
  []
  [boty]
    type = DirichletBC
    variable = disp_y
    boundary = '40'
    value = 0.0
  []
  [topy]
    type = ADFunctionDirichletBC
    variable = disp_y
    boundary = 30
    function = '${starting_point} * cos(2 * pi / 20 * t) + ${offset}'
    preset = false
  []
  [leftx]
    type = ADFunctionDirichletBC
    variable = disp_x
    boundary = 30
    function = '2e-2 * t'
    # function = '0'
    preset = false
  []
[]

[Executioner]
  type = Transient
  end_time = 70
  dt = 0.1 # was 0.5 # TODO
  dtmin = .1
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -pc_svd_monitor '
                  '-snes_linesearch_monitor '
  # -snes_test_jacobian -snes_test_jacobian_view
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -mat_mffd_err'
  petsc_options_value = 'lu       NONZERO               1e-15                   1e-5'
  l_max_its = 30
  nl_max_its = 20
  line_search = 'none'
  snesmf_reuse_base = false
  nl_abs_tol = 1e-9
  nl_rel_tol = 1e-9 # was both 9
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = true
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  active = 'num_nl cumulative_nli contact cumulative_li num_l'
  [num_nl]
    type = NumNonlinearIterations
  []
  [num_l]
    type = NumLinearIterations
  []
  [cumulative_nli]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  []
  [cumulative_li]
    type = CumulativeValuePostprocessor
    postprocessor = num_l
  []
  [contact]
    type = ContactDOFSetSize
    variable = normal_lm
    subdomain = '3'
    execute_on = 'nonlinear timestep_end'
  []
[]
