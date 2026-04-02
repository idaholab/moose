# Base input for testing between-multiapp functor transfers using nearest-position partitioning.

[Problem]
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Positions]
  [input_app1]
    type = InputPositions
    positions = '0 0.1 0
                 0.5 0.5 0'
  []
  [input_app2]
    type = InputPositions
    # offsets to avoid indetermination
    # but small enough to remain below to bounding box factor bump
    positions = '0.0000001 0.30000000001 0
                 0.60000000001 0.5003 0'
  []
[]

[MultiApps/ma1]
  type = TransientMultiApp
  input_files = ../../../nearest_node/nearest_position/sub_between_diffusion.i
  max_procs_per_app = 3
  positions_objects = 'input_app1'
  output_in_position = true
[]

[MultiApps/ma2]
  type = TransientMultiApp
  input_files = ../../../nearest_node/nearest_position/sub_between_diffusion.i
  positions_objects = 'input_app2'
  output_in_position = true
[]

[Transfers]
  [app1_to_2_nodal_nodal]
    type = MultiAppGeneralFieldFunctorTransfer
    from_multi_app = ma1
    to_multi_app = ma2
    source_functors = sent_nodal
    variable = received_nodal
    extrapolation_behavior = nearest-node
    use_nearest_position = input_app1
    bbox_factor = 1.000001
    search_value_conflicts = true
    group_subapps = true
  []
  [app2_to_1_nodal_nodal]
    type = MultiAppGeneralFieldFunctorTransfer
    from_multi_app = ma2
    to_multi_app = ma1
    source_functors = sent_nodal
    variable = received_nodal
    extrapolation_behavior = nearest-node
    use_nearest_position = input_app2
    bbox_factor = 1.000001
    search_value_conflicts = true
    group_subapps = true
  []

  [app1_to_2_elem_elem]
    type = MultiAppGeneralFieldFunctorTransfer
    from_multi_app = ma1
    to_multi_app = ma2
    source_functors = sent_elem
    variable = received_elem
    extrapolation_behavior = nearest-elem
    use_nearest_position = input_app1
    bbox_factor = 1.000001
    search_value_conflicts = true
    group_subapps = true
  []
  [app2_to_1_elem_elem]
    type = MultiAppGeneralFieldFunctorTransfer
    from_multi_app = ma2
    to_multi_app = ma1
    source_functors = sent_elem
    variable = received_elem
    extrapolation_behavior = nearest-elem
    use_nearest_position = input_app2
    bbox_factor = 1.000001
    search_value_conflicts = true
    group_subapps = true
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
