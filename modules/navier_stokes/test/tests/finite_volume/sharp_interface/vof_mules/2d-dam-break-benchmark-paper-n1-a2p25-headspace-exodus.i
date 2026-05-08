!include 2d-dam-break-benchmark-paper-n1-a2p25-headspace.i

[Outputs]
  console := false
  file_base := '2d-dam-break-benchmark-paper-n1-a2p25-headspace-exodus'
  [exo]
    type = Exodus
    file_base = '2d-dam-break-benchmark-paper-n1-a2p25-headspace-exodus'
    show = 'pressure alpha vel_x vel_y'
    time_step_interval = 250
  []
[]
