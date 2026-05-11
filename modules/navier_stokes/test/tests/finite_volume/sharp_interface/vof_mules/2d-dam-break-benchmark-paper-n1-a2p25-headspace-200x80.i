!include 2d-dam-break-benchmark-paper-n1-a2p25.i

# Coarser long-headspace geometry for quick alpha-transport option sweeps.
domain_dims_x := ${fparse 10.0 * a_length}
cell_dx := ${fparse domain_dims_x / 200.0}
domain_dims_y := ${fparse 4.0 * dam_y}
cell_dy := ${fparse domain_dims_y / 80.0}

[Mesh]
  [mesh]
    ix := '200'
    iy := '80'
  []
[]
