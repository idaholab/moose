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
    included_subdomain_ids = '1'
    normal = '1 0 0'
    new_sideset_name = interior
  []
  [transcribe]
    type = ConvertNodeSetSideSetGenerator
    input = sideset
    convert_side_list_from_node_list = false
    convert_node_list_from_side_list = true
  []
[]

[Outputs]
  exodus = true
[]
