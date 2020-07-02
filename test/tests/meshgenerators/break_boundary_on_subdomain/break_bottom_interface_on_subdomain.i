[Mesh]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    xmax = 2
    ny = 2
    ymax = 2
    nz = 2
    zmax = 2
  []

  [./subdomain1]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0 0 0'
    top_right = '1 1 1'
    block_id = 1
  []

  [./subdomain2]
    type = SubdomainBoundingBoxGenerator
    input = subdomain1
    bottom_left = '1 0 0'
    top_right = '2 1 1'
    block_id = 2
  []

  [./interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain2
    primary_block = '1 2'
    paired_block = '0'
    new_boundary = 'interface'
  []

  [./break_boundary]
    input = interface
    type = BreakBoundaryOnSubdomainGenerator
    boundaries = 'bottom interface'
  [../]
[]

[Outputs]
  exodus = true
[]
