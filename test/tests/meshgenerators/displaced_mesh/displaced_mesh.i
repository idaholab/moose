[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [./mg]
    type = AnnularMeshGenerator
    rmin = 1.0
    rmax = 2.0
    nt = 20
  []
[]
