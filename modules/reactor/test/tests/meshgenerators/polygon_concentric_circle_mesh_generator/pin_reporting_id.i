[Mesh]
  [pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '4 4 4 4'
    background_intervals = 2
    polygon_size = 0.63
    polygon_size_style ='apothem'
    ring_radii = '0.2 0.4 0.5'
    ring_intervals = '2 2 1'
    preserve_volumes = on
    flat_side_up = true
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [reporting_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [set_id]
    type = ExtraElementIDAux
    variable = reporting_id
  []
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
