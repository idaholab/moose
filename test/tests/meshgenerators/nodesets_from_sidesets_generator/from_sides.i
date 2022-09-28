[Mesh]
  construct_node_list_from_side_list = false
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
  [subdomains]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'x < 1 & y > 1 & y < 2'
    block_id = 1
  []
  [sideset]
    type = ParsedGenerateSideset
    input = subdomains
    combinatorial_geometry = 'z < 1'
    included_subdomains = '1'
    normal = '1 0 0'
    new_sideset_name = interior
  []
  [add_nodesets]
    type = NodeSetsFromSideSetsGenerator
    input = sideset
  []
[]
