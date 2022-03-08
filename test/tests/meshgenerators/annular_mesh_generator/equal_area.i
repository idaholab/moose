[Mesh]
  [amg]
    type = AnnularMeshGenerator
    nt = 12
    rmin = 1
    rmax = 5
    dmin = 45
    dmax = 135
    equal_area = true
  []
[]

[Outputs]
  exodus = true
[]
