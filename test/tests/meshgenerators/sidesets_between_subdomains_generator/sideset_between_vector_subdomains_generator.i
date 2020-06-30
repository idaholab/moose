[Mesh]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    nz = 0
    zmin = 0
    zmax = 0
    elem_type = QUAD4
  []

  [./subdomain_id]
    type = ElementSubdomainIDGenerator
    input = gmg
    subdomain_ids = '0 0 0 0
                     1 1 1 3
                     2 2 3 3
                     1 2 3 3'
  []

  [./interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain_id
    primary_block = '0 1'
    paired_block = '2 3'
    new_boundary = 'in_between'
  []

  [./emperty_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = interface
    primary_block = '0'
    paired_block = '2'
    new_boundary = 'not_in_mesh'
  []
[]

[Outputs]
  exodus = true
[]
