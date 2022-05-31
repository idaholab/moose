[Mesh]
  construct_node_list_from_side_list = false
  construct_side_list_from_node_list = false
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
    xmax = 3
    ymax = 3
    zmax = 3
  []
  [bounding_box]
    type = BoundingBoxNodeSetGenerator
    input = gmg
    new_boundary = soon_to_be_a_bunch_of_sides
    top_right = '1.1 1.1 1.1'
    bottom_left = '-0.1 -0.1 -0.1'
  []
  [transcribe]
    type = ConvertNodeSetSideSetGenerator
    input = bounding_box
    convert_side_list_from_node_list = true
    convert_node_list_from_side_list = false
  []
[]

[Outputs]
  exodus = true
[]
