[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 3
    ymax = 3
    zmax = 3
    nx = 3
    ny = 3
    nz = 3
  []

  [central_block]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 2
    bottom_left = '1 1 1'
    top_right = '2 2 2'
  []

  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = central_block
    primary_block = 2
    paired_block = 0
    new_boundary = interface
  []

  # Needs the neighbor pointers of the mesh produced by the previous generator
  [external_top]
    type = ParsedGenerateSideset
    input = interface
    combinatorial_geometry = 'z > 2.9'
    new_sideset_name = external_top
    include_only_external_sides = true
    included_subdomains = 0
  []

  # Needs the boundary ids that the previous two generators added
  [interface_top]
    type = ParsedGenerateSideset
    input = external_top
    combinatorial_geometry = 'z > 1.9'
    new_sideset_name = interface_top
    included_boundaries = interface
    excluded_boundaries = external_top
  []
[]

[Outputs]
  exodus = true
[]
