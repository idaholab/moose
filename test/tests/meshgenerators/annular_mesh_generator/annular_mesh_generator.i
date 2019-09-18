[Mesh]
  [./amg]
    type = AnnularMeshGenerator
    nr = 10
    nt = 12
    rmin = 1
    rmax = 5
    tmin = 0.785398163
    tmax = 2.356194490
    growth_r = 1.3
  []
[]

[Outputs]
  exodus = true
[]
