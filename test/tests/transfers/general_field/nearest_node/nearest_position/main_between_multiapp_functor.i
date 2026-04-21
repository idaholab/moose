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
    # Stacked vertically with a gap so the two sub-app domains do not overlap:
    # ma10 covers [0,1]x[0.1,1.1], ma11 covers [-eps,1-eps]x[1.2,2.2]
    # x-offsets avoid indetermination on vertical Voronoi boundaries
    positions = '0 0.1 0
                 -0.0000001 1.2 0'
  []
  [input_app2]
    type = InputPositions
    # Stacked vertically with a gap so the two sub-app domains do not overlap:
    # ma20 covers [eps,1+eps]x[0.3,1.3], ma21 covers [2eps,1+2eps]x[1.4,2.4]
    # x-offsets avoid indetermination on vertical Voronoi boundaries
    positions = '0.0000001 0.30000000001 0
                 0.0000002 1.40000000001 0'
  []
[]

[MultiApps/ma1]
  type = TransientMultiApp
  input_files = sub_between_diffusion.i
  positions_objects = 'input_app1'
  output_in_position = true
[]

[MultiApps/ma2]
  type = TransientMultiApp
  input_files = sub_between_diffusion.i
  positions_objects = 'input_app2'
  output_in_position = true
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Transfers]
  # Nodal to nodal variables
  [app1_to_2_nodal_nodal]
    type = MultiAppGeneralFieldFunctorTransfer
    from_multi_app = ma1
    to_multi_app = ma2
    source_functors = sent_nodal
    variable = received_nodal
    extrapolation_behavior = nearest-node
    use_nearest_position = input_app1
    # slight inflation to avoid floating point issues on borders
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

  # Elemental to elemental variables
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
