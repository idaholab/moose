[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[MeshGenerators]
  [./mg]
    type = AnnularMeshGenerator
    rmin = 1.0
    rmax = 2.0
    nt = 20
  []
[]
