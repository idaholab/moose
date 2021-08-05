[Mesh]
  [./amg]
    type = AnnularMeshGenerator
    nt = 12
    rmin = 1
    rmax = 5
    radial_positions = '2 4'
    dmin = 45
    dmax = 135
  []
[]

[Outputs]
  exodus = true
[]
