starting_point = 2e-1

# We offset slightly so we avoid the case where the bottom of the secondary block and the top of the
# primary block are perfectly vertically aligned which can cause the backtracking line search some
# issues for a coarse mesh (basic line search handles that fine)
offset = 1e-2

[GlobalParams]
  displacements = 'disp_x disp_y'
  diffusivity = 1e0
  correct_edge_dropping = true
[]

[Mesh]
  second_order = true
  [file_mesh]
    type = FileMeshGenerator
    file = long-bottom-block-1elem-blocks-coarse.e
  []
[]

[Variables]
  [disp_x]
    block = '1 2'
    scaling = 1e1
    order = SECOND
  []
  [disp_y]
    block = '1 2'
    scaling = 1e1
    order = SECOND
  []
  [frictional_normal_lm]
    block = 4
    scaling = 1e3
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
  [disp_x]
    type = MatDiffusion
    variable = disp_x
  []
  [disp_y]
    type = MatDiffusion
    variable = disp_y
  []
[]

[UserObjects]
  [weighted_gap_uo]
    type = LMWeightedGapUserObject
    primary_boundary = 10
    secondary_boundary = 20
    primary_subdomain = 3
    secondary_subdomain = 4
    lm_variable = frictional_normal_lm
    disp_x = disp_x
    disp_y = disp_y
    correct_edge_dropping = true
  []
[]
[Constraints]
  [frictional_normal_lm]
    type = ComputeWeightedGapLMMechanicalContact
    primary_boundary = 10
    secondary_boundary = 20
    primary_subdomain = 3
    secondary_subdomain = 4
    variable = frictional_normal_lm
    disp_x = disp_x
    disp_y = disp_y
    normalize_c = true
    c = 1.0e-2
    weighted_gap_uo = weighted_gap_uo
  []
  [normal_x]
    type = NormalMortarMechanicalContact
    primary_boundary = 10
    secondary_boundary = 20
    primary_subdomain = 3
    secondary_subdomain = 4
    variable = frictional_normal_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = weighted_gap_uo
  []
  [normal_y]
    type = NormalMortarMechanicalContact
    primary_boundary = 10
    secondary_boundary = 20
    primary_subdomain = 3
    secondary_subdomain = 4
    variable = frictional_normal_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = weighted_gap_uo
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
    function = '${starting_point} * cos(2 * pi / 40 * t) + ${offset}'
  []
  [leftx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 50
    function = '1e-2 * t'
  []
[]

[Executioner]
  type = Transient
  end_time = 200
  dt = 5
  dtmin = .1
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       NONZERO               1e-15'
  l_max_its = 30
  nl_max_its = 25
  line_search = 'none'
  nl_rel_tol = 1e-12
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
