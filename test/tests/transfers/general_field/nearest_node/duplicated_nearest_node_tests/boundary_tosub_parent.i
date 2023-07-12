[Mesh]
  file = 2blk.e
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left_1]
    type = DirichletBC
    variable = u
    boundary = '1'
    value = 4
  []
  [left_2]
    type = DirichletBC
    variable = u
    boundary = '2'
    value = 3
  []

  [right_3]
    type = DirichletBC
    variable = u
    boundary = '3'
    value = 2
  []
  [right_4]
    type = DirichletBC
    variable = u
    boundary = '4'
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
    positions = '0 -4 0'
    input_files = boundary_tosub_sub.i
  []
[]

[Transfers]
  [to_sub_1]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = sub
    source_variable = u
    from_boundaries = '1'
    variable = from_parent_1
  []
  [to_sub_2]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = sub
    source_variable = u
    from_boundaries = '2'
    variable = from_parent_2
  []
  [to_sub_3]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = sub
    source_variable = u
    from_boundaries = '3'
    variable = from_parent_3
    # Transfer relies on two nodes that are equidistant to the target point
    search_value_conflicts = false
  []
  [to_sub_4]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = sub
    source_variable = u
    from_boundaries = '4'
    variable = from_parent_4
    # Transfer relies on two nodes that are equidistant to the target point
    search_value_conflicts = false
  []
[]
