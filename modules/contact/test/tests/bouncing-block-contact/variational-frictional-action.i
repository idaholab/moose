starting_point = 2e-1

# We offset slightly so we avoid the case where the bottom of the secondary block and the top of the
# primary block are perfectly vertically aligned which can cause the backtracking line search some
# issues for a coarse mesh (basic line search handles that fine)
offset = 1e-2

[GlobalParams]
  displacements = 'disp_x disp_y'
  diffusivity = 1e0
[]

[Mesh]
  [file_mesh]
    type = FileMeshGenerator
    file = long-bottom-block-1elem-blocks-coarse.e
  []
  [remove]
    type = BlockDeletionGenerator
    input = file_mesh
    block = '3 4'
  []
  patch_update_strategy = iteration
[]

# [Problem]
#   type = DumpObjectsProblem
#   dump_path = Contact/contact_action
# []

[Variables]
  [disp_x]
    block = '1 2'
    scaling = 1e1
  []
  [disp_y]
    block = '1 2'
    scaling = 1e1
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

[AuxVariables]
  [procid]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [procid]
    type = ProcessorIDAux
    variable = procid
  []
[]

[Contact]
  [contact_action]
    model = coulomb
    formulation = mortar
    c_normal = 1.0e-2
    c_tangential = 1.0e-1
    friction_coefficient = 0.1
    primary = 10
    secondary = 20
    normalize_c = true
    normal_lm_scaling = 1e3
    tangential_lm_scaling = 1e2
    correct_edge_dropping = true
    use_dual = false
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
  [exodus]
    type = Exodus
    hide = 'procid contact_pressure nodal_area penetration'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]
