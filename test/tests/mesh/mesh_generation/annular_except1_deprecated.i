# Exception testing of AnnularMesh.
# In this file rmax < rmin
[Mesh]
  type = AnnularMesh
  nr = 10
  nt = 12
  rmin = 5
  rmax = 1
  tmin = 45
  tmax = 135
  growth_r = 1.3
[]
