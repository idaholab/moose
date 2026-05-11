!include 2d-dam-break-benchmark-paper-n1-a2p25-headspace-200x80.i

# Option 3: Van Leer alpha correction, lower alpha Courant, and two minimum subcycles.
c_alpha := 0.1

[Physics]
  [NavierStokes]
    [SharpInterfaceVOFSegregated]
      [vof]
        alpha_correction_scheme = 'vanLeer'
      []
    []
  []
[]

[Executioner]
  dt := 3.0e-4
  adjust_momentum_pressure_time_step := true
  momentum_pressure_max_courant := 5.0

  volume_fraction_subcycles := 2
  volume_fraction_max_courant := 0.5
[]

[Outputs]
  console := false
  file_base := '2d-dam-break-benchmark-paper-n1-a2p25-headspace-200x80-opt3-vanleer-lowalphaco-sub2-exodus'
  [exo]
    type = Exodus
  file_base := '2d-dam-break-benchmark-paper-n1-a2p25-headspace-200x80-opt3-vanleer-lowalphaco-sub2-exodus'
    show = 'pressure alpha vel_x vel_y'
    time_step_interval = 50
  []
[]
