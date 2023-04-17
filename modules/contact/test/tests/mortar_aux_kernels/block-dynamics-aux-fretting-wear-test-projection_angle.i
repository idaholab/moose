starting_point = 0.5e-1
offset = -0.045

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = long-bottom-block-1elem-blocks-multiple-projections-lowerd.e
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
  #  scaling = 1.0e-5
  []
  [frictional_lm]
    block = 3
    use_dual = true
  #  scaling = 1.0e-5
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

[Kernels]
  [DynamicTensorMechanics]
    displacements = 'disp_x disp_y'
    generate_output = 'stress_xx stress_yy'
    strain = FINITE
    block = '1 2'
    zeta = 1.0
    hht_alpha = 0.0
  []
  [inertia_x]
    type = InertialForce
    variable = disp_x
    velocity = vel_x
    acceleration = accel_x
    beta = 0.25
    gamma = 0.5
    alpha = 0
    eta = 0.0
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
    eta = 0.0
    block = '1 2'
  []
[]

[Materials]
  [elasticity_2]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [elasticity_1]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e8
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

[AuxVariables]
  [worn_depth]
    block = '3'
  []
  [gap_vel]
    block = '3'
  []
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
  [gap_vel]
    type = WeightedGapVelAux
    variable = gap_vel
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    disp_x = disp_x
    disp_y = disp_y
    debug_mesh = true
    minimum_projection_angle = 0.0
  []
  [worn_depth]
    type = MortarArchardsLawAux
    variable = worn_depth
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    displacements = 'disp_x disp_y'
    friction_coefficient = 0.5
    energy_wear_coefficient = 1.0e-6
    normal_pressure = normal_lm
    execute_on = 'TIMESTEP_END'
    debug_mesh = true
    minimum_projection_angle = 0.0
  []
  [accel_x]
    type = NewmarkAccelAux
    variable = accel_x
    displacement = disp_x
    velocity = vel_x
    beta = 0.25
    execute_on = 'linear timestep_end'
  []
  [vel_x]
    type = NewmarkVelAux
    variable = vel_x
    acceleration = accel_x
    gamma = 0.5
    execute_on = 'linear timestep_end'
  []
  [accel_y]
    type = NewmarkAccelAux
    variable = accel_y
    displacement = disp_y
    velocity = vel_y
    beta = 0.25
    execute_on = 'linear timestep_end'
  []
  [vel_y]
    type = NewmarkVelAux
    variable = vel_y
    acceleration = accel_y
    gamma = 0.5
    execute_on = 'linear timestep_end'
  []
[]

[UserObjects]
  [weighted_vel_uo]
    type = LMWeightedVelocitiesUserObject
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    secondary_variable = disp_x
    lm_variable_normal = normal_lm
    lm_variable_tangential_one = frictional_lm
    disp_x = disp_x
    disp_y = disp_y
    debug_mesh = true
  []
[]

[Constraints]
  [weighted_gap_lm]
    type = ComputeDynamicFrictionalForceLMMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    variable = normal_lm
    disp_x = disp_x
    disp_y = disp_y
    use_displaced_mesh = true
    wear_depth = worn_depth
    c = 1e6
    c_t = 1e6
    normalize_c = true
    mu = 0.5
    friction_lm = frictional_lm
    capture_tolerance = 1.0e-5
    newmark_beta = 0.25
    newmark_gamma = 0.5
    debug_mesh = true
    minimum_projection_angle = 0.0
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
    debug_mesh = true
    minimum_projection_angle = 0.0
    weighted_gap_uo = weighted_vel_uo
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
    debug_mesh = true
    minimum_projection_angle = 0.0
    weighted_gap_uo = weighted_vel_uo
  []
  [tangential_x]
    type = TangentialMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    variable = frictional_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    debug_mesh = true
    minimum_projection_angle = 0.0
    weighted_velocities_uo = weighted_vel_uo
  []
  [tangential_y]
    type = TangentialMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    variable = frictional_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    debug_mesh = true
    minimum_projection_angle = 0.0
    weighted_velocities_uo = weighted_vel_uo
  []
[]

[BCs]
  [botx]
    type = DirichletBC
    variable = disp_x
    boundary = 40
    value = 0.0
  []
  [boty]
    type = DirichletBC
    variable = disp_y
    boundary = 40
    value = 0.0
  []
  [topy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 30
    function = '${starting_point} * cos(4.0 * pi / 4 * t) + ${offset}'
  []
  [leftx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 50
    function = '1e-2 * (cos(32.0 * pi / 4 * t) - 1.0)'
  []
[]

[Executioner]
  type = Transient
  end_time = 0.0
  dt = 0.05
  dtmin = .002
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -pc_svd_monitor '
                  '-snes_linesearch_monitor -snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       superlu_dist                  NONZERO               1e-15'
  nl_max_its = 40
  l_max_its = 15
  line_search = 'l2'
  snesmf_reuse_base = true

  [TimeIntegrator]
    type = NewmarkBeta
    beta = 0.25
    gamma = 0.5
  []
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = true
  checkpoint = true
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  active = 'num_nl cumulative contact'
  [num_nl]
    type = NumNonlinearIterations
  []
  [cumulative]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  []
  [contact]
    type = ContactDOFSetSize
    variable = normal_lm
    subdomain = '3'
    execute_on = 'nonlinear timestep_end'
  []
[]
