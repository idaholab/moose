!include 2d-dam-break-benchmark-paper-n1-a2p25-headspace.i

c_alpha := 0.1

[Executioner]
  # Start from a larger nominal global step and let the executioner reduce it
  # to keep the momentum/pressure Courant number under the requested cap.
  dt := 3.0e-4
  adjust_momentum_pressure_time_step := true
  momentum_pressure_max_courant := 5.0

  # Start from one alpha solve per global step and let the executioner add
  # subcycles so the transported alpha Courant number per substep stays below 1.
  volume_fraction_subcycles := 1
  volume_fraction_max_courant := 0.9
[]

[Outputs]
  console := false
  file_base := '2d-dam-break-benchmark-paper-n1-a2p25-headspace-high-compression-exodus'
  [exo]
    type = Exodus
    file_base = '2d-dam-break-benchmark-paper-n1-a2p25-headspace-high-compression-exodus'
    show = 'pressure alpha vel_x vel_y'
    time_step_interval = 50
  []
[]
