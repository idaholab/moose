[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    xmax = 2
    ny = 4
    ymax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 1
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain1
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'interface'
  []
  [layer]
    type = BoundaryLayerSubdomainGenerator
    input = 'interface'
    boundaries = 'interface'
    block_name = 'new'
  []
[]
