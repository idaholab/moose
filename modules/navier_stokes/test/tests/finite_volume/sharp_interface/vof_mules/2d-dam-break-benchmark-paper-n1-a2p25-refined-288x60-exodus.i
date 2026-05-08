!include 2d-dam-break-benchmark-paper-n1-a2p25.i

cell_dx := ${fparse domain_dims_x / 288.0}
cell_dy := ${fparse domain_dims_y / 60.0}

[Mesh]
  [mesh]
    ix := '288'
    iy := '60'
  []
[]

[Outputs]
  console := false
  file_base := '2d-dam-break-benchmark-paper-n1-a2p25-refined-288x60-exodus'
  [exo]
    type = Exodus
    file_base = '2d-dam-break-benchmark-paper-n1-a2p25-refined-288x60-exodus'
    show = 'pressure alpha vel_x vel_y'
    time_step_interval = 250
  []
[]
