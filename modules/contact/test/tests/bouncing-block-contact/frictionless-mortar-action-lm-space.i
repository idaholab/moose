starting_point = 2e-1
offset = 1e-2

[GlobalParams]
  displacements = 'disp_x disp_y'
  diffusivity = 1e0
  scaling = 1e0
[]

[Mesh]
  [file_mesh]
    type = FileMeshGenerator
    file = long-bottom-block-1elem-blocks.e
  []
  [second_order]
    type = ElementOrderConversionGenerator
    input = file_mesh
    conversion_type = SECOND_ORDER
  []
  [remove]
    type = BlockDeletionGenerator
    input = second_order
    block = '3 4'
  []
  patch_update_strategy = always
[]

[Variables]
  [disp_x]
    block = '1 2'
    order = SECOND
  []
  [disp_y]
    block = '1 2'
    order = SECOND
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

[Contact]
  [contact_action]
    formulation = mortar
    model = frictionless
    primary = 20
    secondary = 10
    c_normal = 1
    use_dual = false
  []
[]

[BCs]
  [botx]
    type = DirichletBC
    variable = disp_x
    boundary = 40
    value = 0.0
    preset = false
  []
  [boty]
    type = DirichletBC
    variable = disp_y
    boundary = 40
    value = 0.0
    preset = false
  []
  [topy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 30
    function = '${starting_point} * cos(2 * pi / 40 * t) + ${offset}'
    preset = false
  []
  [leftx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 50
    function = '1e-2 * t'
    preset = false
  []
[]

[Executioner]
  type = Transient
  end_time = 0
  dt = 1
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       NONZERO               1e-15'
  line_search = 'none'
[]

[Outputs]
  exodus = false
  csv = true
  checkpoint = false
  file_base = frictionless-mortar-action-lm-space-match-displacement
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  active = 'nl_dofs aux_dofs'
  [nl_dofs]
    type = NumDOFs
    system = NL
    execute_on = INITIAL
  []
  [aux_dofs]
    type = NumDOFs
    system = AUX
    execute_on = INITIAL
  []
[]
