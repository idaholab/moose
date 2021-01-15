[Mesh]
  [./amg]
    type = AnnularMeshGenerator
    nr = 10
    nt = 12
    rmin = 1
    rmax = 5
    dmin = 45
    dmax = 135
    growth_r = -1.3
  []
[]

[Outputs]
  exodus = true
[]
