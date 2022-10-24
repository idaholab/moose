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
  [u_elemental]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [fun_aux]
    type = FunctionAux
    function = 'x + y'
    variable = u_elemental
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
    execute_on = timestep_end
    positions = '0.48 0.01 0'
    input_files = tosub_sub.i
  []
[]

[Transfers]
  [to_sub_nodal_to_nodal]
    type = MultiAppGeneralFieldNearestNodeTransfer
    to_multi_app = sub
    source_variable = u
    variable = nodal_source_from_parent_nodal
  []
  [to_sub_nodal_to_elemental]
    type = MultiAppGeneralFieldNearestNodeTransfer
    to_multi_app = sub
    source_variable = u
    variable = nodal_source_from_parent_elemental
  []
  [to_sub_elemental_to_nodal]
    type = MultiAppGeneralFieldNearestNodeTransfer
    to_multi_app = sub
    source_variable = u_elemental
    variable = elemental_source_from_parent_nodal
  []
  [to_sub_elemental_to_elemental]
    type = MultiAppGeneralFieldNearestNodeTransfer
    to_multi_app = sub
    source_variable = u_elemental
    variable = elemental_source_from_parent_elemental
  []
[]
