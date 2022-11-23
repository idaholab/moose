[Mesh]
  construct_side_list_from_node_list = false  # set to false for testing purposes
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
    new_boundary = 'nodes_to_convert_to_sides'
    top_right = '2.1 2.1 1.1'
    bottom_left = '-0.1 -0.1 -0.1'
  []
  [create_sideset]
    type = SideSetsFromNodeSetsGenerator
    input = bounding_box
  []
[]
