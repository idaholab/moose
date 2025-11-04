[Problem]
  solve = false
[]

Box2_inactive_id = '3'
Box2_inactive_name = 'Box2_inactive'
inactive_domain_block_ids = ${Box2_inactive_id}
inactive_domain_block_names = ${Box2_inactive_name}

[Mesh]
  [BaseMesh]
    type = GeneratedMeshGenerator
    elem_type = TET4
    dim = 3
    nx = 5
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
    top_right = "20 20 2"
  []
  [Box2]
    type = SubdomainBoundingBoxGenerator
    input = "Box1"
    block_id = 2
    location = "INSIDE"
    bottom_left = "-2 -2 2"
    top_right = "2 2 0"
  []
  add_subdomain_ids = ${inactive_domain_block_ids}
  add_subdomain_names = ${inactive_domain_block_names}
[]

[AuxVariables]
  [dummy]
    type = MooseVariableFVReal
  []
[]

# move elements between subdomains back and forth
[MeshModifiers]
  [GlobalSubdomainModifier]
    type = TimedSubdomainModifier
    header = ON
    data_file = 'tsm.csv'
    comment = '#'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  end_time = 1.0
[]

[Outputs]
  exodus = true
[]
