# This generates the mesh used by the other tests.
[Mesh]
  [annular]
    type = AnnularMeshGenerator
    nr = 5
    rmin = 0.1
    rmax = 0.2
    growth_r = 1
    nt = 5
    dmin = 0
    dmax = 90
  []
  [make3D]
    type = MeshExtruderGenerator
    extrusion_vector = '0 0 1'
    num_layers = 10
    bottom_sideset = 'bottom'
    top_sideset = 'top'
    input = annular
  []
[]
