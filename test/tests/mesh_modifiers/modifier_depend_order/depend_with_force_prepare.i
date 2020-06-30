[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 1
  nz = 10
  xmin = -1000
  xmax = 1000
  ymin = -1
  ymax = 1
  zmin = -400
  zmax = 0
[]

[MeshModifiers]
  [./add_subdomain]
    type = SubdomainBoundingBox
    block_id = 1
    bottom_left = '-200 -1 -400'
    top_right = '200 1 0'
    force_prepare = true
  [../]
  [./add_sidesets_between]
    type = SideSetsBetweenSubdomains
    primary_block = 1
    paired_block = 0
    depends_on = add_subdomain
    new_boundary = 10
  [../]
  [./add_sidesets_around]
    type = SideSetsAroundSubdomain
    block = 1
    depends_on = add_subdomain
    new_boundary = 11
  [../]
[]
