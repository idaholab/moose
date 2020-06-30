[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  nz = 0
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[MeshModifiers]
  [./subdomain_id]
    type = AssignElementSubdomainID
    subdomain_ids = '0 0 0 0
                     1 1 1 3
                     2 2 3 3
                     1 2 3 3'
  [../]
  [./interface]
    type = SideSetsBetweenSubdomains
    depends_on = 'subdomain_id'
    primary_block = '0 1'
    paired_block = '2 3'
    new_boundary = 'in_between'
  [../]
  [./emperty_interface]
    type = SideSetsBetweenSubdomains
    depends_on = 'subdomain_id'
    primary_block = '0'
    paired_block = '2'
    new_boundary = 'not_in_mesh'
  [../]
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
