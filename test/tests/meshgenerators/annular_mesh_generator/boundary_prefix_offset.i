[Mesh]
  [amg]
    type = AnnularMeshGenerator
    nt = 12
    rmin = 1
    rmax = 5
    dmin = 45
    dmax = 135
    boundary_id_offset = 7
    boundary_name_prefix = bunga
  []
  [rename]
    type = RenameBoundaryGenerator
    input = amg
    old_boundary = '7 bunga_rmax'
    new_boundary = 'little_john khan'
  []
[]

[Outputs]
  exodus = true
[]
