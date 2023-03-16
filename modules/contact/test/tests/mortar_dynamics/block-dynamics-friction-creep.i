starting_point = 1e-1
offset = -0.095

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = long-bottom-block-1elem-blocks.e
[]

[Variables]
  [normal_lm]
    block = 3
    use_dual = true
  []
  [frictional_lm]
    block = 3
    use_dual = true
  []
[]

[AuxVariables]
  [creep_strain_xx]
    order = CONSTANT
    family = MONOMIAL
    block = '2'
  []
  [creep_strain_yy]
    order = CONSTANT
    family = MONOMIAL
    block = '2'
  []
  [creep_strain_xy]
    order = CONSTANT
    family = MONOMIAL
    block = '2'
  []
[]
[AuxKernels]
  [creep_strain_xx]
    type = RankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_xx
    index_i = 0
    index_j = 0
    block = '2'
  []
  [creep_strain_yy]
    type = RankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_yy
    index_i = 1
    index_j = 1
    block = '2'
  []
  [creep_strain_xy]
    type = RankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_xy
    index_i = 0
    index_j = 1
    block = '2'
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

[Modules/TensorMechanics/DynamicMaster]
  [all]
    add_variables = true
    alpha = 0.0
    newmark_beta = 0.25
    newmark_gamma = 0.5
    mass_damping_coefficient = 0.0
    stiffness_damping_coefficient = 0.01
    displacements = 'disp_x disp_y'
    generate_output = 'stress_xx stress_yy'
    block = '1 2'
    strain = FINITE
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
  [multiple_inelastic]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'creep'
    block = '2'
  []
  [creep]
    type = PowerLawCreepStressUpdate
    coefficient = 1.0e-23 # 10e-24
    n_exponent = 4
    activation_energy = 0
    block = '2'
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = '1'
  []
  [density]
    type = GenericConstantMaterial
    block = '1 2'
    prop_names = 'density'
    prop_values = '775'
  []
[]

# User object provides the contact force (e.g. LM)
# for the application of the generalized force
[UserObjects]
  [weighted_vel_uo]
    type = LMWeightedVelocitiesUserObject
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    lm_variable_normal = normal_lm
    lm_variable_tangential_one = frictional_lm
    secondary_variable = disp_x
    disp_x = disp_x
    disp_y = disp_y
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
    friction_lm = frictional_lm
    disp_x = disp_x
    disp_y = disp_y
    use_displaced_mesh = true
    c = 1e4
    c_t = 1e4
    mu = 0.5
    interpolate_normals = false
    newmark_beta = 0.25
    newmark_gamma = 0.5
    capture_tolerance = 1e-04
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
    function = '${starting_point} * cos(2 * pi / 4 * t) + ${offset}'
  []
  [leftx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 30 # 50
    function =  '1e-2*t' #'0.1 *sin(2 * pi / 12 * t)'
  []
[]

[Executioner]
  type = Transient
  end_time = 0.25
  dt = 0.05
  dtmin = 0.05
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -pc_svd_monitor '
                  '-snes_linesearch_monitor -snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -mat_mffd_err '
  petsc_options_value = 'lu       NONZERO               1e-15                   1e-5'
  nl_max_its = 50
  line_search = 'none'
  snesmf_reuse_base = false

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
