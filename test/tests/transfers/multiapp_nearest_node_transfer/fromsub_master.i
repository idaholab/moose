[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  elem_type = QUAD8
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[AuxVariables]
  [nodal_source_from_sub_nodal]
    family = LAGRANGE
    order = FIRST
  []
  [nodal_source_from_sub_elemental]
    family = MONOMIAL
    order = CONSTANT
  []
  [elemental_source_from_sub_nodal]
    family = LAGRANGE
    order = FIRST
  []
  [elemental_source_from_sub_elemental]
    family = MONOMIAL
    order = CONSTANT
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

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0.48 0.01 0 -1.01 0.01 0'
    input_files = fromsub_sub.i
  []
[]

[Transfers]
  [from_sub_nodal_from_nodal]
    type = MultiAppNearestNodeTransfer
    from_multi_app = sub
    source_variable = u
    variable = nodal_source_from_sub_nodal
  []
  [from_sub_nodal_from_elemental]
    type = MultiAppNearestNodeTransfer
    from_multi_app = sub
    source_variable = u
    variable = nodal_source_from_sub_elemental
  []
  [from_sub_elemental_from_nodal]
    type = MultiAppNearestNodeTransfer
    from_multi_app = sub
    source_variable = u_elemental
    variable = elemental_source_from_sub_nodal
  []
  [from_sub_elemental_from_elemental]
    type = MultiAppNearestNodeTransfer
    from_multi_app = sub
    source_variable = u_elemental
    variable = elemental_source_from_sub_elemental
  []
[]
