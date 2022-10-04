[Mesh]
  [gen]
    type = GeneratedMeshGenerator
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

  [add_subdomain]
    type = SubdomainBoundingBoxGenerator
    input = gen
    block_id = 1
    bottom_left = '-200 -1 -400'
    top_right = '200 1 0'
    force_prepare = true
  []
  [add_sidesets_around]
    type = SideSetsAroundSubdomainGenerator
    input = add_subdomain
    block = 1
    new_boundary = 11
  []
[]
