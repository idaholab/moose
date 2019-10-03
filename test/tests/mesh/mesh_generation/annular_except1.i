# Exception testing of AnnularMesh.
# In this file rmax < rmin
[Mesh]
  type = AnnularMesh
  nr = 10
  nt = 12
  rmin = 5
  rmax = 1
  dmin = 45
  dmax = 135
  growth_r = 1.3
[]
