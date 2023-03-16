[Mesh]
  [generated_mesh]
    type = FileMeshGenerator
    file = half_sphere.e
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 2002
    new_block_name = 'secondary_lower'
    sidesets = '202'
    input = generated_mesh
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 1002
    sidesets = '102'
    new_block_name = 'primary_lower'
    input = secondary
  []
  patch_size = 20
  patch_update_strategy = always
  uniform_refine = 0
[]

[Problem]
  kernel_coverage_check = false
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
  order = FIRST
  family = LAGRANGE
[]

[Variables]
  [frictional_normal_lm]
    block = 'secondary_lower'
    use_dual = true
  []
  [frictional_tangential_lm]
    block = 'secondary_lower'
    use_dual = true
  []
  [frictional_tangential_dir_lm]
    block = 'secondary_lower'
    use_dual = true
  []
[]

[AuxVariables]
  [saved_x]
  []
  [saved_y]
  []
  [saved_z]
  []
  [tangent_x]
    family = LAGRANGE
    order = FIRST
  []
  [tangent_y]
    family = LAGRANGE
    order = FIRST
  []
  [tangent_z]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxKernels]
  [friction_x_component]
   type = MortarFrictionalPressureVectorAux
   primary_boundary = 102
   secondary_boundary = 202
   tangent_one = frictional_tangential_lm
   tangent_two = frictional_tangential_dir_lm
   variable = tangent_x
   component = 0
   boundary = 202
  []
  [friction_y_component]
   type = MortarFrictionalPressureVectorAux
   primary_boundary = 102
   secondary_boundary = 202
   tangent_one = frictional_tangential_lm
   tangent_two = frictional_tangential_dir_lm
   variable = tangent_y
   component = 1
   boundary = 202
  []
  [friction_z_component]
   type = MortarFrictionalPressureVectorAux
   primary_boundary = 102
   secondary_boundary = 202
   tangent_one = frictional_tangential_lm
   tangent_two = frictional_tangential_dir_lm
   variable = tangent_z
   component = 2
   boundary = 202
  []
[]

[Functions]
  [push_down]
    type = ParsedFunction
    expression = 'if(t < 1.5, -t, t-3.0)'
  []
  [force_z]
    type = ParsedFunction
    expression = 'if(t < 0.008, 0.0, (-t)*2.0e2 -t*t*100.0)' # 4.0e5
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    block = '1 2'
    use_automatic_differentiation = true
    generate_output = 'stress_xx stress_xy stress_xz stress_yy stress_zz'
    save_in = 'saved_x saved_y saved_z'
    use_finite_deform_jacobian = true
  []
[]

[BCs]
  [botz]
    type = ADDirichletBC
    variable = disp_z
    boundary = 101
    value = 0.0
  []
  [boty]
    type = ADDirichletBC
    variable = disp_y
    boundary = 101
    value = 0.0
  []
  [botx]
    type = ADDirichletBC
    variable = disp_x
    boundary = 101
    value = 0.0
  []

  [topz]
    type = ADFunctionDirichletBC
    variable = disp_z
    boundary = '201'
    function = push_down
  []
  [topy]
    type = ADDirichletBC
    variable = disp_y
    boundary = '201 202'
    value = 0.0
  []
  [topx]
    type = ADDirichletBC
    variable = disp_x
    boundary = '201 202'
    value = 0.0
  []
[]

[Materials]
  [tensor]
    type = ADComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1.40625e7
    poissons_ratio = 0.25
  []
  [stress]
    type = ADComputeFiniteStrainElasticStress
    block = '1'
  []

  [tensor_1000]
    type = ADComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1e5
    poissons_ratio = 0.0
  []
  [stress_1000]
    type = ADComputeFiniteStrainElasticStress
    block = '2'
  []
[]

[Postprocessors]
  [stress_zz]
    type = ElementAverageValue
    variable = stress_zz
    block = 1
  []
  [resid_z]
    type = NodalSum
    variable = saved_z
    boundary = 201
  []
  [disp_z]
    type = NodalExtremeValue
    variable = disp_z
    boundary = 201
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -pc_factor_shift_type '
                        '-pc_factor_shift_amount -mat_mffd_err'
  petsc_options_value = 'lu    superlu_dist nonzero 1e-14 1e-5'

  line_search = 'none'
  l_max_its = 60
  nl_max_its = 50
  dt = 0.004
  dtmin = 0.00001
  # end_time = 1.8
  end_time = 0.000
  nl_rel_tol = 1.0e-6 #1e-7 # -8, -6 to avoid many iterations. Switch it March 2021
  nl_abs_tol = 1e-6 # 6 if no friction
  l_tol = 1e-4
[]

[Outputs]
  exodus = true
  csv = true
  print_linear_residuals = true
  perf_graph = true
  [console]
    type = Console
    max_rows = 5
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[UserObjects]
  [weighted_vel_uo]
    type = LMWeightedVelocitiesUserObject
    primary_boundary = 102
    secondary_boundary = 202
    primary_subdomain = 1002
    secondary_subdomain = 2002
    lm_variable_normal = frictional_normal_lm
    lm_variable_tangential_one = frictional_tangential_lm
    lm_variable_tangential_two = frictional_tangential_dir_lm
    secondary_variable = disp_x
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    debug_mesh = true
  []
[]

[Constraints]
  [weighted_gap_lm]
    type = ComputeFrictionalForceLMMechanicalContact
    primary_boundary = 102
    secondary_boundary = 202
    primary_subdomain = 1002
    secondary_subdomain = 2002
    variable = frictional_normal_lm
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    use_displaced_mesh = true
    friction_lm = frictional_tangential_lm
    friction_lm_dir = frictional_tangential_dir_lm
    c = 7.0e4
    c_t = 7.0e4
    mu = 0.4
    debug_mesh = true
    weighted_gap_uo = weighted_vel_uo
    weighted_velocities_uo = weighted_vel_uo
  []
  [normal_x]
    type = NormalMortarMechanicalContact
    primary_boundary = 102
    secondary_boundary = 202
    primary_subdomain = 1002
    secondary_subdomain = 2002
    variable = frictional_normal_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    debug_mesh = true
    weighted_gap_uo = weighted_vel_uo
  []
  [normal_y]
    type = NormalMortarMechanicalContact
    primary_boundary = 102
    secondary_boundary = 202
    primary_subdomain = 1002
    secondary_subdomain = 2002
    variable = frictional_normal_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    debug_mesh = true
    weighted_gap_uo = weighted_vel_uo
  []
  [normal_z]
    type = NormalMortarMechanicalContact
    primary_boundary = 102
    secondary_boundary = 202
    primary_subdomain = 1002
    secondary_subdomain = 2002
    variable = frictional_normal_lm
    secondary_variable = disp_z
    component = z
    use_displaced_mesh = true
    compute_lm_residuals = false
    debug_mesh = true
    weighted_gap_uo = weighted_vel_uo
  []
  [tangential_x]
    type = TangentialMortarMechanicalContact
    primary_boundary = 102
    secondary_boundary = 202
    primary_subdomain = 1002
    secondary_subdomain = 2002
    variable = frictional_tangential_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    debug_mesh = true
    weighted_velocities_uo = weighted_vel_uo
  []
  [tangential_y]
    type = TangentialMortarMechanicalContact
    primary_boundary = 102
    secondary_boundary = 202
    primary_subdomain = 1002
    secondary_subdomain = 2002
    variable = frictional_tangential_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    debug_mesh = true
    weighted_velocities_uo = weighted_vel_uo
  []
  [tangential_z]
    type = TangentialMortarMechanicalContact
    primary_boundary = 102
    secondary_boundary = 202
    primary_subdomain = 1002
    secondary_subdomain = 2002
    variable = frictional_tangential_lm
    secondary_variable = disp_z
    component = z
    use_displaced_mesh = true
    compute_lm_residuals = false
    debug_mesh = true
    weighted_velocities_uo = weighted_vel_uo
  []
  [tangential_x_dir]
    type = TangentialMortarMechanicalContact
    primary_boundary = 102
    secondary_boundary = 202
    primary_subdomain = 1002
    secondary_subdomain = 2002
    variable = frictional_tangential_dir_lm
    secondary_variable = disp_x
    component = x
    direction = direction_2
    use_displaced_mesh = true
    compute_lm_residuals = false
    debug_mesh = true
    weighted_velocities_uo = weighted_vel_uo
  []
  [tangential_y_dir]
    type = TangentialMortarMechanicalContact
    primary_boundary = 102
    secondary_boundary = 202
    primary_subdomain = 1002
    secondary_subdomain = 2002
    variable = frictional_tangential_dir_lm
    secondary_variable = disp_y
    component = y
    direction = direction_2
    use_displaced_mesh = true
    compute_lm_residuals = false
    debug_mesh = true
    weighted_velocities_uo = weighted_vel_uo
  []
  [tangential_z_dir]
    type = TangentialMortarMechanicalContact
    primary_boundary = 102
    secondary_boundary = 202
    primary_subdomain = 1002
    secondary_subdomain = 2002
    variable = frictional_tangential_dir_lm
    secondary_variable = disp_z
    component = z
    direction = direction_2
    use_displaced_mesh = true
    compute_lm_residuals = false
    debug_mesh = true
    weighted_velocities_uo = weighted_vel_uo
  []
[]

[Debug]
 show_var_residual_norms = true
[]
