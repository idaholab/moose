[Mesh]
  construct_node_list_from_side_list = false  # these are set to false for demonstration purposes
  construct_side_list_from_node_list = false  #
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
    type = NodeSetToSideSetGenerator
    input = bounding_box
  []
[] # ordinarily the construct_side_list_from_node_list method is called after the mesh generation

[Outputs]
  exodus = true
[]
