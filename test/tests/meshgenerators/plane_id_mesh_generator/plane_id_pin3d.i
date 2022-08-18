[Mesh]
  [pin2d]
    type = ConcentricCircleMeshGenerator
    num_sectors = 2
    radii = '0.4 0.5'
    rings = '1 1 1'
    has_outer_square = on
    pitch = 1.26
    preserve_volumes = yes
    smoothing_max_it = 3
  []
  [pin3d]
    type = AdvancedExtruderGenerator
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
