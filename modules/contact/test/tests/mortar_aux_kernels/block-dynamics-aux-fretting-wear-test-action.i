starting_point = 0.5e-1
offset = -0.045

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = long-bottom-block-1elem-blocks.e
  []
  [remote]
    type = BlockDeletionGenerator
    input = file
    block = '3 4'
  []
[]

[Variables]
  [disp_x]
    block = '1 2'
  []
  [disp_y]
    block = '1 2'
  []
[]

[Problem]
  material_coverage_check = false
  kernel_coverage_check = false
[]

[ICs]
  [disp_y]
    block = 2
    variable = disp_y
    value = '${fparse starting_point + offset}'
    type = ConstantIC
  []
[]

[Physics/SolidMechanics/Dynamic]
  [all]
    hht_alpha = 0.0
    newmark_beta = 0.25
    newmark_gamma = 0.5
    mass_damping_coefficient = 0.0
    stiffness_damping_coefficient = 1.0
    accelerations = 'accel_x accel_y'
    generate_output = 'stress_xx stress_yy'
    block = '1 2'
    strain = FINITE
    density = density
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
  [density]
    type = GenericConstantMaterial
    block = '1 2'
    prop_names = 'density'
    prop_values = '7750'
  []
[]

[AuxVariables]
  [worn_depth]
    block = 'normal_secondary_subdomain'
  []
  [gap_vel]
    block = 'normal_secondary_subdomain'
  []
  [vel_x]
    order = FIRST
    family = LAGRANGE
  []
  [vel_y]
    order = FIRST
    family = LAGRANGE
  []
  [accel_x]
    order = FIRST
    family = LAGRANGE
  []
  [accel_y]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxKernels]
  [gap_vel]
    type = WeightedGapVelAux
    variable = gap_vel
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = normal_primary_subdomain
    secondary_subdomain = normal_secondary_subdomain
    disp_x = disp_x
    disp_y = disp_y
  []
  [worn_depth]
    type = MortarArchardsLawAux
    variable = worn_depth
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = normal_primary_subdomain
    secondary_subdomain = normal_secondary_subdomain
    displacements = 'disp_x disp_y'
    friction_coefficient = 0.5
    energy_wear_coefficient = 1.0e-6
    normal_pressure = normal_normal_lm
    execute_on = 'TIMESTEP_END'
  []
[]

[Contact]
  [normal]
    formulation = mortar
    model = coulomb
    primary = 20
    secondary = 10
    c_normal = 1e+06
    c_tangential = 1.0e+6
    capture_tolerance = 1.0e-5
    newmark_beta = 0.25
    newmark_gamma = 0.5
    mortar_dynamics = true
    wear_depth = worn_depth
    friction_coefficient = 0.5
    normalize_c = true
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
  end_time = 0.5
  dt = 0.05
  dtmin = .002
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -pc_svd_monitor '
                  '-snes_linesearch_monitor -snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type '
                        '-pc_factor_shift_amount'
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
    variable = normal_normal_lm
    subdomain = '3'
    execute_on = 'nonlinear timestep_end'
  []
[]
