# Exception testing of AnnularMesh.
# In this file tri_subdomain_id = quad_subdomain_id
[Mesh]
  type = AnnularMesh
  nr = 10
  nt = 3
  rmin = 0
  rmax = 5
  quad_subdomain_id = 1
  tri_subdomain_id = 1
  growth_r = 1.3
[]
