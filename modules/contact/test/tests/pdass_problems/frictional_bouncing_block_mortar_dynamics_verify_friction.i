starting_point = 2e-1
offset = 1e-2

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  file = long-bottom-block-1elem-blocks.e
  uniform_refine = 0 # 1,2
  patch_update_strategy = always
[]

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Variables]
  [disp_x]
    block = '1 2'
  []
  [disp_y]
    block = '1 2'
  []
  [frictional_normal_lm]
    block = 3
    use_dual = true
  []
  [frictional_tangential_lm]
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

# [Modules/TensorMechanics/Master]
#   [all]
#     strain = FINITE
#     generate_output = 'stress_xx stress_yy'
#     block = '1 2'
#   []
# []

[Functions]
  [push]
    type = ParsedFunction
    value = 'if(t < 10, (2e-1 * cos(2 * pi / 20 * t) + 1e-2) , (2e-1 * cos(2 * pi / 20 * 10.0) + '
            '1e-2))'
  []
  # [tangential]
  #   type = ParsedFunction
  #   value = 'if(t < 10, 0, 0.0)'
  # []
  [tangential]
    type = ParsedFunction
    value = 'if(t < 10, 0, 0.2*(t-10))'
  []
[]

[Kernels]
  [DynamicTensorMechanics]
    displacements = 'disp_x disp_y'
    generate_output = 'stress_xx stress_yy'
    # strain = FINITE
    block = '1 2'
    zeta = 0.1
    alpha = 0.0
  []
  [inertia_x]
    type = InertialForce
    variable = disp_x
    velocity = vel_x
    acceleration = accel_x
    beta = 0.25
    gamma = 0.5
    alpha = 0
    eta = 0.1
    block = '1 2'
  []
  [inertia_y]
    type = InertialForce
    variable = disp_y
    velocity = vel_y
    acceleration = accel_y
    beta = 0.25
    gamma = 0.5
    alpha = 0
    eta = 0.1
    block = '1 2'
  []
[]

[AuxVariables]
  [vel_x]
    block = '1 2'
  []
  [accel_x]
    block = '1 2'
  []
  [vel_y]
    block = '1 2'
  []
  [accel_y]
    block = '1 2'
  []
  [vel_z]
    block = '1 2'
  []
  [accel_z]
    block = '1 2'
  []
[]

[AuxKernels]
  [accel_x]
    type = NewmarkAccelAux
    variable = accel_x
    displacement = disp_x
    velocity = vel_x
    beta = 0.25
    execute_on = 'LINEAR timestep_end'
  []
  [vel_x]
    type = NewmarkVelAux
    variable = vel_x
    acceleration = accel_x
    gamma = 0.5
    execute_on = 'LINEAR timestep_end'
  []
  [accel_y]
    type = NewmarkAccelAux
    variable = accel_y
    displacement = disp_y
    velocity = vel_y
    beta = 0.25
    execute_on = 'LINEAR timestep_end'
  []
  [vel_y]
    type = NewmarkVelAux
    variable = vel_y
    acceleration = accel_y
    gamma = 0.5
    execute_on = 'LINEAR timestep_end'
  []
[]

[Constraints]
  [weighted_gap_lm]
    type = ComputeDynamicFrictionalForceLMMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    variable = frictional_normal_lm
    disp_x = disp_x
    disp_y = disp_y
    use_displaced_mesh = true
    friction_lm = frictional_tangential_lm
    mu = 0.1
    c = 1.0e4
    c_t = 5.0e1
  []
  # [weighted_gap_lm]
  #   type = ComputeDynamicWeightedGapLMMechanicalContact
  #   primary_boundary = 20
  #   secondary_boundary = 10
  #   primary_subdomain = 4
  #   secondary_subdomain = 3
  #   variable = frictional_normal_lm
  #   disp_x = disp_x
  #   disp_y = disp_y
  #   use_displaced_mesh = true
  #   c = 1.0e0
  # []
  [normal_x]
    type = NormalMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    variable = frictional_normal_lm
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
    variable = frictional_normal_lm
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
    variable = frictional_tangential_lm
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
    variable = frictional_tangential_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
  []
[]

[Materials]
  [elasticity_2]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1e4
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
  [strain]
    type = ComputeFiniteStrain
    block = '1 2'
  []
  [density]
    type = GenericConstantMaterial
    block = '1 2'
    prop_names = 'density'
    prop_values = '7750'
  []
[]

[BCs]
  [botx]
    type = DirichletBC
    variable = disp_x
    boundary = '40'
    value = 0.0
    preset = false
  []
  [boty]
    type = DirichletBC
    variable = disp_y
    boundary = '40'
    value = 0.0
    preset = false
  []
  [topy]
    type = ADFunctionDirichletBC
    variable = disp_y
    boundary = 30
    function = 'push'
    preset = false
  []
  [leftx]
    type = ADFunctionDirichletBC
    variable = disp_x
    boundary = 30
    function = 'tangential'
    # function = '0'
    preset = false
  []
[]

[Executioner]
  type = Transient
  end_time = 20 # 70
  dt = 0.1 # 0.1 for finer meshes (uniform_refine)
  dtmin = .01
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -pc_svd_monitor '
                  '-snes_linesearch_monitor '
  petsc_options_iname = '-pc_type -pc_factor_shift_type '
  petsc_options_value = 'lu       NONZERO               '
  l_max_its = 30
  nl_max_its = 40
  line_search = 'none'
  snesmf_reuse_base = false
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  l_tol = 1e-07 # Tightening l_tol can help with friction
  scheme = newmark-beta
  automatic_scaling = true
  off_diagonals_in_auto_scaling = true
[]

[Debug]
  show_var_residual_norms = true
[]

[VectorPostprocessors]
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
  active = 'num_nl cumulative_nli cumulative_li num_l'
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
[]

# Time step doesn't affect (actually, smaller worse)
# Preconditioner doesn't affect
# Damping doesn't affect contact area (of course, it does affect material)
# Preset BCs?
# Why initial step residual is so high?
# Are Mortar constraints correctly included in Newmark-Beta scheme?
