[Mesh]
  [assm_up]
    type = TriPinHexAssemblyGenerator
    ring_radii = '7 8;5 6; '
    ring_intervals = '2 1;1 1; '
    ring_block_ids = '200 400 600;700 800; '
    background_block_ids = '30 40'
    num_sectors_per_side = 6
    background_intervals = 2
    hexagon_size = ${fparse 40.0/sqrt(3.0)}
    ring_offset = 0.6
    external_boundary_id = 200
    external_boundary_name = 'surface'
    ring_id_name = 'ring_id'
    sector_id_name = 'sector_id'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = Exodus
    output_extra_element_ids = true
    extra_element_ids_to_output = 'ring_id sector_id'
    execute_on = timestep_end
  []
[]
