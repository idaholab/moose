[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[MeshDivisions]
  [quadrants]
    type = CartesianGridDivision
    bottom_left = '-.01 -.01 0'
    top_right = '1.02 1.02 0'
    nx = 2
    ny = 2
    nz = 1
  []
[]

[AuxVariables]
  [from_sub]
    initial_condition = -1
  []
  [from_sub_elem]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = -1
  []
  [to_sub]
    [InitialCondition]
      type = FunctionIC
      function = '1 + 2*x*x + 3*y*y*y'
    []
  []
  [to_sub_elem]
    order = CONSTANT
    family = MONOMIAL
    [InitialCondition]
      type = FunctionIC
      function = '2 + 2*x*x + 3*y*y*y'
    []
  []
  [mesh_div]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [mesh_div]
    type = MeshDivisionAux
    mesh_division = quadrants
    variable = mesh_div
    execute_on = 'initial'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    app_type = MooseTestApp
    input_files = sub_corner.i
    output_in_position = true
    # Placed at the four corners so each sub-app sits within one quadrant of 'quadrants'
    positions = '-.04 -.04 0
                 0.94 -.04 0
                 -.04 0.94 0
                 0.94 0.94 0'
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppGeneralFieldFunctorTransfer
    to_multi_app = sub
    source_functors = to_sub
    variable = from_main
    extrapolation_behavior = nearest-node
    from_mesh_division = quadrants
    from_mesh_division_usage = 'matching_subapp_index'
    bbox_factor = 20
    search_value_conflicts = true
  []

  [to_sub_elem]
    type = MultiAppGeneralFieldFunctorTransfer
    to_multi_app = sub
    source_functors = to_sub_elem
    variable = from_main_elem
    extrapolation_behavior = nearest-elem
    from_mesh_division = quadrants
    from_mesh_division_usage = 'matching_subapp_index'
    bbox_factor = 20
    search_value_conflicts = true
  []

  [from_sub]
    type = MultiAppGeneralFieldFunctorTransfer
    from_multi_app = sub
    source_functors = to_main
    variable = from_sub
    extrapolation_behavior = nearest-node
    to_mesh_division = quadrants
    to_mesh_division_usage = 'matching_subapp_index'
    bbox_factor = 20
    search_value_conflicts = true
  []

  [from_sub_elem]
    type = MultiAppGeneralFieldFunctorTransfer
    from_multi_app = sub
    source_functors = to_main_elem
    variable = from_sub_elem
    extrapolation_behavior = nearest-elem
    to_mesh_division = quadrants
    to_mesh_division_usage = 'matching_subapp_index'
    bbox_factor = 20
    search_value_conflicts = true
  []
[]
