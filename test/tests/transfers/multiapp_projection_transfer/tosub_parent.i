[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 9
  ymin = 0
  ymax = 9
  nx = 9
  ny = 9
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [x]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Functions]
  [x_func]
    type = ParsedFunction
    expression = x
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[AuxKernels]
  [x_func_aux]
    type = FunctionAux
    variable = x
    function = x_func
    execute_on = initial
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1

  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]

[Debug]
#  show_actions = true
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    execute_on = timestep_end
    positions = '1 1 0 5 5 0'
    input_files = tosub_sub.i
  []
[]

[Transfers]
  [tosub]
    type = MultiAppProjectionTransfer
    to_multi_app = sub
    source_variable = u
    variable = u_nodal
  []
  [elemental_tosub]
    type = MultiAppProjectionTransfer
    to_multi_app = sub
    source_variable = u
    variable = u_elemental
  []
  [elemental_to_sub_elemental]
    type = MultiAppProjectionTransfer
    to_multi_app = sub
    source_variable = x
    variable = x_elemental
  []
  [elemental_to_sub_nodal]
    type = MultiAppProjectionTransfer
    to_multi_app = sub
    source_variable = x
    variable = x_nodal
  []
[]
