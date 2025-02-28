[Problem]
  solve = false
[]
[Mesh]
  [block_one]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 1
    xmin = 4.5
    xmax = 5.5
    ymin = 4.5
    ymax = 5.5
    zmin = 0.001
    zmax = 1.001
    boundary_name_prefix = 'ball'
    elem_type = TET4
  []
  [block_two]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 10
    nz = 1
    xmin = 3
    xmax = 7
    ymin = 3
    ymax = 7
    zmin = -2
    zmax = 0
    boundary_name_prefix = 'base'
    boundary_id_offset = 10
    elem_type = TET4
  []
  [block_one_id]
    type = SubdomainIDGenerator
    input = block_one
    subdomain_id = 1
  []
  [block_two_id]
    type = SubdomainIDGenerator
    input = block_two
    subdomain_id = 2
  []
  [combine]
    type = MeshCollectionGenerator
    inputs = ' block_one_id block_two_id'
  []
[]

[AuxVariables]
  [penetration]
    order = FIRST
    family = LAGRANGE
  []
  [closest_point_x]
    order = FIRST
    family = LAGRANGE
  []
  [closest_point_y]
    order = FIRST
    family = LAGRANGE
  []
  [closest_point_z]
    order = FIRST
    family = LAGRANGE
  []
  [tang_dist]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxKernels]
  [penetrate]
    type = PenetrationAux
    variable = penetration
    boundary = base_front
    paired_boundary = ball_back
    quantity = distance
  []

  [close_x]
    type = PenetrationAux
    variable = closest_point_x
    boundary = base_front
    paired_boundary = ball_back
    quantity = closest_point_x
  []

  [close_y]
    type = PenetrationAux
    variable = closest_point_y
    boundary = base_front
    paired_boundary = ball_back
    quantity = closest_point_y
  []

  [close_z]
    type = PenetrationAux
    variable = closest_point_z
    boundary = base_front
    paired_boundary = ball_back
    quantity = closest_point_z
  []
  [tang_dist]
    type = PenetrationAux
    variable = tang_dist
    boundary = base_front
    paired_boundary = ball_back
    quantity = tangential_distance
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  execute_on = TIMESTEP_END
[]
