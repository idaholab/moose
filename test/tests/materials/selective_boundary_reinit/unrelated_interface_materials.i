[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
  []
  [blocks]
    type = SubdomainPerElementGenerator
    input = generate
    subdomain_ids = '0 1'
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = blocks
    primary_block = 0
    paired_block = 1
    new_boundary = interface
  []
[]

[Materials]
  [volume]
    type = BoundaryMaterialReinitTest
    property = volume_property
    error_on_face = true
    error_on_neighbor = true
  []
  [boundary]
    type = BoundaryMaterialReinitTest
    property = boundary_property
    boundary = interface
    error_on_boundary = true
  []
[]

[UserObjects]
  [interface]
    type = MaterialReinitInterfaceUserObject
    boundary = interface
    execute_on = TIMESTEP_END
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1
[]
