[Mesh]
  [file]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1 1'
    dy = '1 1'
    subdomain_id = '1 2 3 4
                    1 2 3 4'
  []

  [extrude]
    type = AdvancedExtruderGenerator
    direction = '0 0 1'
    heights = '1 1 1'
    num_layers = '1 1 1'
    input = file
  []

  [add_side]
    type = ParsedGenerateSideset
    combinatorial_geometry = '2 > 1'
    new_sideset_name = new_s
    included_subdomain_ids = 1
    included_neighbor_ids = 2
    input = extrude
  []
[]
