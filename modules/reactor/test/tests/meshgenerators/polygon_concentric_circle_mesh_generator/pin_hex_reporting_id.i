[Mesh]
  [pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    background_intervals = 2
    polygon_size = 0.63
    polygon_size_style ='apothem'
    ring_radii = '0.2 0.4 0.5'
    ring_intervals = '2 2 1'
    sector_id_name = 'sector_id'
    ring_id_name = 'ring_id'
    preserve_volumes = on
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = timestep_end
    output_extra_element_ids = true
    extra_element_ids_to_output = 'ring_id sector_id'
  []
[]
