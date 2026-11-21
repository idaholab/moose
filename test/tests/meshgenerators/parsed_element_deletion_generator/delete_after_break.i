[Mesh]
  [msh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1 1'
    dy = '1 1'
    ix = '1 1 1 1'
    iy = '1 1'
    subdomain_id = '0 1 2 3
                    0 1 2 3'
  []
  [split]
    input = msh
    type = BreakMeshByBlockGenerator
    block_pairs = '0 1;
                   2 3'
  []
  # Removes the whole 0-1 pair; partially deleting a split pair is an error in find_neighbors
  [cut]
    type = ParsedElementDeletionGenerator
    input = split
    expression = 'x < 2'
  []
  # Needs the neighbors across the cut and across the remaining 2-3 split interface
  [exterior]
    type = ParsedGenerateSideset
    input = cut
    combinatorial_geometry = 'x > 0'
    new_sideset_name = exterior
    include_only_external_sides = true
  []
[]

[Outputs]
  exodus = true
[]
