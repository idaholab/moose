# Base input for testing functor transfers restricted by mesh divisions matched to subapp indices.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[MeshDivisions]
  [middle]
    type = CartesianGridDivision
    bottom_left = '0.21 0.21 0'
    top_right = '1.001 1.001 0'
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
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  [out]
    type = Exodus
    hide = 'to_sub to_sub_elem div'
    overwrite = true
  []
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    app_type = MooseTestApp
    input_files = ../../../nearest_node/mesh_division/sub.i
    output_in_position = true
    positions = '0.1001 0.0000013 0
                 0.30054 0.600001985 0
                 0.70021 0.4000022 0
                 0.800212 0.8500022 0'
    cli_args = 'base_value=1 base_value=2 base_value=3 base_value=4'
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppGeneralFieldFunctorTransfer
    to_multi_app = sub
    source_functors = to_sub
    variable = from_main
    extrapolation_behavior = nearest-node
    from_mesh_division = middle
    from_mesh_division_usage = 'matching_subapp_index'
  []

  [to_sub_elem]
    type = MultiAppGeneralFieldFunctorTransfer
    to_multi_app = sub
    source_functors = to_sub_elem
    variable = from_main_elem
    extrapolation_behavior = nearest-elem
    from_mesh_division = middle
    from_mesh_division_usage = 'matching_subapp_index'
  []

  [from_sub]
    type = MultiAppGeneralFieldFunctorTransfer
    from_multi_app = sub
    source_functors = to_main
    variable = from_sub
    extrapolation_behavior = nearest-node
    to_mesh_division = middle
    to_mesh_division_usage = 'matching_subapp_index'
  []

  [from_sub_elem]
    type = MultiAppGeneralFieldFunctorTransfer
    from_multi_app = sub
    source_functors = to_main_elem
    variable = from_sub_elem
    extrapolation_behavior = nearest-elem
    to_mesh_division = middle
    to_mesh_division_usage = 'matching_subapp_index'
  []
[]

[AuxVariables]
  [div]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [mesh_div]
    type = MeshDivisionAux
    variable = div
    mesh_division = 'middle'
  []
[]
