[Mesh]
  [assm_up]
    type = TriPinHexAssemblyGenerator
    ring_radii = '7 8'
    ring_intervals = '2 1'
    ring_block_ids = '200 400 600'
    background_block_ids = '40'
    num_sectors_per_side = 6
    background_intervals = 2
    hexagon_size = ${fparse 40.0/sqrt(3.0)}
    ring_offset = 0.6
    assembly_orientation = pin_down
  []
[]
