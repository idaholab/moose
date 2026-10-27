[Mesh]
  [base]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 2
    xmax = 4
    ymax = 2
    elem_type = TRI3
  []
  # Split the mesh in two blocks so we can place an internal sideset between them
  [top_block]
    type = ParsedSubdomainMeshGenerator
    input = 'base'
    combinatorial_geometry = 'y > 1'
    block_id = 1
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'top_block'
    primary_block = 0
    paired_block = 1
    new_boundary = 'interface'
  []
  [coarsen]
    type = CoarsenSurfaceMeshAlongSidesetGenerator
    input = 'interface'
    boundaries = 'interface'
    verbose = true
  []
[]

[Outputs]
  exodus = true
[]
