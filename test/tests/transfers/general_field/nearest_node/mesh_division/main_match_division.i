# Base input for testing transfers with mesh divisions restrictions with a mapping from
# source mesh divisions to target mesh divisions. It has the following complexities:
# - multiple sub-applications
# - transfers both from and to the subapps
# - both nodal and elemental variables
# Tests derived from this input may add complexities through command line arguments

# Explaining results on the main app:
# Each value on the main app comes from a region with the same division index on a subapp
# Because the subapp is not very discretized in Y, some source mesh division indices
# are not represented in the subapps. Therefore these mesh divisions are not present in the
# main app.
# Explaining results on the sub apps:
# Each subapp receives results for all its target mesh divisions. They are naturally all the
# same because they are all matched with the same source app (parent app) source division
# and the division on the parent app is too small to have more than 1 valid point + value

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[MeshDivisions]
  [middle]
    type = CartesianGridDivision
    bottom_left = '0.21 0.21 0'
    # cover more and sample more bins
    top_right = '1.001 1.001 0'
    nx = 5
    ny = 5
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
    input_files = sub.i
    output_in_position = true
    # The positions are randomly offset to prevent equi-distant nearest-locations
    positions = '0.1001 0.0000013 0
                 0.30054 0.600001985 0
                 0.70021 0.0000022 0
                 0.800212 0.5500022 0'
    cli_args = "base_value=1;MeshDivisions/middle_sub/nx=5;MeshDivisions/middle_sub/ny=5;Mesh/nx=10 "
               "base_value=2;MeshDivisions/middle_sub/nx=5;MeshDivisions/middle_sub/ny=5;Mesh/nx=10 "
               "base_value=3;MeshDivisions/middle_sub/nx=5;MeshDivisions/middle_sub/ny=5;Mesh/nx=10 "
               "base_value=4;MeshDivisions/middle_sub/nx=5;MeshDivisions/middle_sub/ny=5;Mesh/nx=10"
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = sub
    source_variable = to_sub
    variable = from_main
    from_mesh_division = middle
    from_mesh_division_usage = 'matching_division'
    to_mesh_division = middle_sub
    to_mesh_division_usage = 'matching_division'
    # we avoid bounding boxes because the parent and children apps do
    # not overlap so unless we grow the boxes to cover the entire source,
    # the transfer will not pick up all the source mesh divisions
    greedy_search = true
    use_bounding_boxes = false
  []

  [to_sub_elem]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = sub
    source_variable = to_sub_elem
    variable = from_main_elem
    from_mesh_division = middle
    from_mesh_division_usage = 'matching_division'
    to_mesh_division = middle_sub
    to_mesh_division_usage = 'matching_division'
    greedy_search = true
    use_bounding_boxes = false
  []

  [from_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    source_variable = to_main
    variable = from_sub
    from_mesh_division = middle_sub
    from_mesh_division_usage = 'matching_division'
    to_mesh_division = middle
    to_mesh_division_usage = 'matching_division'
    greedy_search = true
    use_bounding_boxes = false
  []

  [from_sub_elem]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    source_variable = to_main_elem
    variable = from_sub_elem
    from_mesh_division = middle_sub
    from_mesh_division_usage = 'matching_division'
    to_mesh_division = middle
    to_mesh_division_usage = 'matching_division'
    greedy_search = true
    use_bounding_boxes = false
  []
[]

# For debugging purposes
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
