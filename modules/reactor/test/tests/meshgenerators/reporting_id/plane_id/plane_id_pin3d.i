[Mesh]
  [pin2d]
    type = PolygonConcentricCircleMeshGenerator
    preserve_volumes = true
    ring_radii = 0.4
    ring_intervals = 2
    background_intervals = 1
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    polygon_size = 0.5
  []
  [pin3d]
    type = FancyExtruderGenerator
    input = 'pin2d'
    heights = '5.0 5.0 5.0'
    direction = '0 0 1'
    num_layers = '2 2 2'
  []
  [pin3d_id]
    type = PlaneIDMeshGenerator
    input = 'pin3d'
    plane_coordinates = '0.0 5.0 10.0 15.0'
    num_ids_per_plane = ' 1 2 1'
    plane_axis = 'z'
    id_name = 'plane_id'
  []
[]


[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [plane_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [set_plane_id]
    type = ExtraElementIDAux
    variable = plane_id
    extra_id_name = plane_id
  []
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
