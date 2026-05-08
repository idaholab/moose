!include 2d-dam-break-benchmark-paper-n1-a2p25.i

# Add substantial gas headspace above the initial column and extend the domain
# in x while preserving the baseline cell sizes. This keeps the atmospheric
# outlet and the downstream boundary well away from the dam-break motion.
domain_dims_x := ${fparse 10.0 * a_length}
cell_dx := ${fparse domain_dims_x / 400.0}
domain_dims_y := ${fparse 4.0 * dam_y}
cell_dy := ${fparse domain_dims_y / 160.0}

[Mesh]
  [mesh]
    ix := '400'
    iy := '160'
  []
[]
