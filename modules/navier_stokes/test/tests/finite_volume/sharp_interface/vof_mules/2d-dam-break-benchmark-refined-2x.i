!include 2d-dam-break-benchmark.i

cell_dx := ${fparse domain_dims_x / 400.0}
cell_dy := ${fparse domain_dims_y / 100.0}

[Mesh]
  [mesh]
    ix := '400'
    iy := '100'
  []
[]
