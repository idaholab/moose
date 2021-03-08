[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 4
  ny = 4
  nz = 4
[]

[Variables]
  [dummy]
  []
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = dummy
  []
[]

[AuxVariables]
  [u]
  []
  [w_from_sub_left_to_left]
    family = MONOMIAL
    order = CONSTANT
  []
  [w_from_sub_top_to_top]
    family = MONOMIAL
    order = CONSTANT
  []
  [w_from_sub_right_to_bottom]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = u
  []
[]

[Functions]
  [u]
    type = ParsedFunction
    value = 'x+y+z'
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    input_files = 'boundary_elemental_sub.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [u_to_sub_left_to_left]
    type = MultiAppNearestNodeTransfer
    direction = to_multiapp
    multi_app = sub
    source_variable = u
    variable = u_from_master_left_to_left
    source_boundary = 'left'
    target_boundary = 'left'
  []
  [u_to_sub_top_to_top]
    type = MultiAppNearestNodeTransfer
    direction = to_multiapp
    multi_app = sub
    source_variable = u
    variable = u_from_master_top_to_top
    source_boundary = 'top'
    target_boundary = 'top'
  []
  [u_to_sub_right_to_bottom]
    type = MultiAppNearestNodeTransfer
    direction = to_multiapp
    multi_app = sub
    source_variable = u
    variable = u_from_master_right_to_bottom
    source_boundary = 'right'
    target_boundary = 'bottom'
  []
  [w_from_sub_left_to_left]
    type = MultiAppNearestNodeTransfer
    direction = from_multiapp
    multi_app = sub
    source_variable = w
    variable = w_from_sub_left_to_left
    source_boundary = 'left'
    target_boundary = 'left'
  []
  [w_from_sub_top_to_top]
    type = MultiAppNearestNodeTransfer
    direction = from_multiapp
    multi_app = sub
    source_variable = w
    variable = w_from_sub_top_to_top
    source_boundary = 'top'
    target_boundary = 'top'
  []
  [w_from_sub_right_to_bottom]
    type = MultiAppNearestNodeTransfer
    direction = from_multiapp
    multi_app = sub
    source_variable = w
    variable = w_from_sub_right_to_bottom
    source_boundary = 'right'
    target_boundary = 'bottom'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
  hide = dummy
[]
