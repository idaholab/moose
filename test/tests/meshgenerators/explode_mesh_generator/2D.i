[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 2
    ymax = 2
    nx = 4
    ny = 4
  []
  [add_subdomain_1]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '1 0 0'
    top_right = '2 1 0'
  []
  [add_subdomain_2]
    type = SubdomainBoundingBoxGenerator
    input = add_subdomain_1
    block_id = 2
    bottom_left = '1 1 0'
    top_right = '2 2 0'
  []
  [add_subdomain_3]
    type = SubdomainBoundingBoxGenerator
    input = add_subdomain_2
    block_id = 3
    bottom_left = '0 1 0'
    top_right = '1 2 0'
  []
  [explode]
    type = ExplodeMeshGenerator
    input = add_subdomain_3
    subdomains = '1 2'
    interface_name = czm
  []
[]
