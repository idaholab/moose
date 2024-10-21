[Problem]
  solve = false
[]

Box2_inactive_id = '4'
Box3_inactive_id = '5'
Box2_inactive_name = 'Box2_inactive'
Box3_inactive_name = 'Box3_inactive'
inactive_domain_block_ids = '${Box2_inactive_id} ${Box3_inactive_id}'
inactive_domain_block_names = '${Box2_inactive_name} ${Box3_inactive_name}'

[Mesh]
  [BaseMesh]
    type = GeneratedMeshGenerator
    elem_type = TET4
    dim = 3
    nx = 4
    ny = 3
    nz = 2
    xmin = -10
    xmax = 10
    ymin = -10
    ymax = 10
    zmin = -2
    zmax = 2
  []
  [Box1]
    type = SubdomainBoundingBoxGenerator
    input = "BaseMesh"
    block_id = 1
    location = "INSIDE"
    bottom_left = "-20 -20 -2"
    top_right = "20 20 +2"
  []
  [Box2]
    type = SubdomainBoundingBoxGenerator
    input = "Box1"
    block_id = 2
    location = "INSIDE"
    bottom_left = "-4 -3 3"
    top_right = "0 3 0"
  []
  [Box3]
    type = SubdomainBoundingBoxGenerator
    input = "Box2"
    block_id = 3
    location = "INSIDE"
    bottom_left = "0 -3 2"
    top_right = "4 3 0"
  []
  add_subdomain_ids = ${inactive_domain_block_ids}
  add_subdomain_names = ${inactive_domain_block_names}
[]

[AuxVariables]
  [dummy]
    type = MooseVariableFVReal
  []
[]

[MeshModifiers]
  [GlobalSubdomainModifier]
    type = TimedSubdomainModifier
    times = '      0.4            0.6  0.4'
    blocks_from = '2              4    3'
    blocks_to = '  Box2_inactive  2    Box3_inactive' # Subdomain names are permitted ('Box2_inactive' = 4, etc)
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Executioner]
  dt = 0.1
  type = Transient
  end_time = 1.0
[]

[Outputs]
  exodus = true
[]
