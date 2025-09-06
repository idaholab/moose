[Mesh]
  construct_side_list_from_node_list = false  # set to false for testing purposes
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
  [included]
    type = ParsedGenerateNodeset
    input = 'gmg'
    new_nodeset_name = 'included'
    expression = 'x>0.4999 & x<0.5001'
  []
  [not_included]
    type = ParsedGenerateNodeset
    input = 'included'
    new_nodeset_name = 'not_included'
    expression = 'y>0.4999 & y<0.5001'
  []
  [create_sideset]
    type = SideSetsFromNodeSetsGenerator
    input = not_included
    nodesets_to_convert = 'included'
  []
[]
