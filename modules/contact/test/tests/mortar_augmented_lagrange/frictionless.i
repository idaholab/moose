starting_point = 0# 2e-1
offset = 0#1e-2

[GlobalParams]
  displacements = 'disp_x disp_y'
  diffusivity = 1e0
  scaling = 1e0
  preset = false
[]

[Mesh]
  file = ../bouncing-block-contact/long-bottom-block-1elem-blocks.e
[]

[Problem]
  type = AugmentedLagrangianContactFEProblem
[]

[Variables]
  [disp_x]
    block = '1 2'
  []
  [disp_y]
    block = '1 2'
  []
[]

# [ICs]
#   [disp_y]
#     block = 2
#     variable = disp_y
#     value = '${fparse starting_point + offset}'
#     type = ConstantIC
#   []
# []

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
    type = PenaltyWeightedGapUserObject
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    disp_x = disp_x
    disp_y = disp_y
    penalty = 5e1
    penetration_tolerance = 1e-2
  []
[]

[Constraints]
  [normal_x]
    type = NormalMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = weighted_gap_uo
  []
  [normal_y]
    type = NormalMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
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
    function = '${starting_point} + ${offset} - t/50'
  []
  [leftx]
    type = DirichletBC
    variable = disp_x
    boundary = 50
    value = 0
  []
[]

[Executioner]
  type = Transient
  end_time = 35
  dt = 1
  dtmin = .1
  solve_type = NEWTON
  petsc_options = '-snes_converged_reason -ksp_converged_reason -pc_svd_monitor '
                  '-snes_linesearch_monitor'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu      '
  l_max_its = 30
  nl_max_its = 1000
  line_search = 'none'
  snesmf_reuse_base = true
  abort_on_solve_fail = true
  nl_rel_tol = 1e-12
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = true
  csv = true
  perf_graph = true
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[AuxVariables]
  [gap]
  []
[]

[AuxKernels]
  [gap]
    type = PenaltyMortarUserObjectAux
    variable = gap
    user_object = weighted_gap_uo
    contact_quantity = weighted_gap
  []
[]

[Postprocessors]
  [num_nl]
    type = NumNonlinearIterations
  []
  [cumulative]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  []
  [gap]
    type = SideExtremeValue
    value_type = min
    variable = gap
    boundary = 10
  []
[]
