[Mesh]
  construct_side_list_from_node_list = true
  construct_side_list_from_node_list_exterior_only = true
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 1
  []
  [both_faces]
    type = ParsedGenerateNodeset
    input = gmg
    new_nodeset_name = 'both_faces'
    expression = 'z<0.0001 | z>0.9999'
    allow_distributed_meshes = true
  []
[]
