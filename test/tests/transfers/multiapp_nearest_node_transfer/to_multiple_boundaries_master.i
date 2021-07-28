# Master mesh and sub mesh are same with 4x4 quad8 elements.
# master mesh has top boundary fixed at u=2 and bottom fixed at u=-1
# sub mesh has top boundary fixed at u = 2 and bottom fixed at u=1
# The u variable is transferred to the left and bottom boundaries of the sub,
# while the v variable is transferred to the right and top boundaries of the master.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  elem_type = QUAD8
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [from_sub]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 2.0
  []
  [bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = -1.0
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1

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
    input_files = to_multiple_boundaries_sub.i
    execute_on = timestep_end
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppNearestNodeTransfer
    direction = to_multiapp
    multi_app = sub
    source_variable = u
    target_boundary = 'left bottom'
    variable = from_master
  []
  [from_sub]
    type = MultiAppNearestNodeTransfer
    direction = from_multiapp
    multi_app = sub
    source_variable = v
    target_boundary = 'right top'
    variable = from_sub
  []
[]
