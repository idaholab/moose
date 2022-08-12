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

[AuxVariables]
  [ring_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [sector_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [set_ring_id]
    type = ExtraElementIDAux
    variable = ring_id
    extra_id_name = ring_id
  []
  [set_sector_id]
    type = ExtraElementIDAux
    variable = sector_id
    extra_id_name = sector_id
  []
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
