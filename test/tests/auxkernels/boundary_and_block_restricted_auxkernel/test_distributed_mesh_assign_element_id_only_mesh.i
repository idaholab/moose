[Mesh]
    type = GeneratedMesh
    dim = 2
    xmin = -1
    ymin = -1
    xmax = 1
    ymax = 1
    nx = 2
    ny = 2
    elem_type = QUAD4
  []

[MeshModifiers]
  [./subdomain_id]
    type = AssignElementSubdomainID
    subdomain_ids = '1 1
                     0 1'
  [../]

  [./interface]
    type = SideSetsBetweenSubdomains
    depends_on = subdomain_id
    master_block = '0'
    paired_block = '1'
    new_boundary = 'interface'
  [../]
[]
