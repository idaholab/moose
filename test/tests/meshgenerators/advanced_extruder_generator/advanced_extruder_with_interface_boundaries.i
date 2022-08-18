[Mesh]
  # See fancy_extruder_with_boundary_swap.i for details about mesh_2d.e
  [fmg]
    type = FileMeshGenerator
    file = mesh_2d.e
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = fmg
    heights = '1 2 3'
    num_layers = '1 2 1'
    direction = '0 0 1'
    bottom_boundary = '100'
    top_boundary = '200'
    subdomain_swaps = '1 11 2 12 3 13;
                       1 21 2 22 3 23;
                       1 31 2 32 3 33'
    upward_boundary_source_blocks = '1;2 3;1 2 3'
    upward_boundary_ids = '1001;2002 2003;3001 3002 3003'
    downward_boundary_source_blocks = '1 2 3;2 3;1'
    downward_boundary_ids = '1501 1502 1503;2502 2503;3501'
  []
[]
