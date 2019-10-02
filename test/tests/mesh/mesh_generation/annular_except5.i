# Exception testing of AnnularMesh.
# In this file nt < (dmax - dmin) / Pi
[Mesh]
  type = AnnularMesh
  nr = 10
  nt = 1
  rmin = 1
  rmax = 5
  dmax = 230
  growth_r = 1.3
[]
