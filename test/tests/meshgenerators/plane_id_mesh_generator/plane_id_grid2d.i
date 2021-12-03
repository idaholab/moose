[Mesh]
  [grid]
    type = CartesianMeshGenerator
    dim = 2
    dx = '5.0 10.0 '
    ix = '1 4'
    dy = '5.0 10.0 '
    iy = '1 4'
  []

  [plane_id_gen]
    type = PlaneIDMeshGenerator
    input = 'grid'
    plane_coordinates = '0.0 5.0 15.0'
    num_ids_per_plane = ' 1 2'
    plane_axis = 'x'
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
