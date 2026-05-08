!include 2d-dam-break-benchmark-paper-n1-a2p25-headspace.i

c_alpha := 0.5

[Executioner]
  # Use adaptive global stepping and alpha subcycling so this run is directly
  # comparable to the recent high-compression headspace experiments.
  dt := 3.0e-4
  adjust_momentum_pressure_time_step := true
  momentum_pressure_max_courant := 5.0

  volume_fraction_subcycles := 1
  volume_fraction_max_courant := 0.9
[]

[Outputs]
  console := false
  file_base := '2d-dam-break-benchmark-paper-n1-a2p25-headspace-alpha05-exodus'
  [exo]
    type = Exodus
    file_base = '2d-dam-break-benchmark-paper-n1-a2p25-headspace-alpha05-exodus'
    show = 'pressure alpha vel_x vel_y'
    time_step_interval = 50
  []
[]
