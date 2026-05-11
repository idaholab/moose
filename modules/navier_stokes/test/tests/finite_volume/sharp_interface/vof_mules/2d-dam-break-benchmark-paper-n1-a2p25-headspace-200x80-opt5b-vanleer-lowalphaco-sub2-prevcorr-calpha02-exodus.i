!include 2d-dam-break-benchmark-paper-n1-a2p25-headspace-200x80.i

# Option 5 sweep high end: as option 4, but with a larger compression factor.
c_alpha := 0.2

[Physics]
  [NavierStokes]
    [SharpInterfaceVOFSegregated]
      [vof]
        alpha_correction_scheme = 'vanLeer'
        alpha_apply_prev_corr := true
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
  file_base := '2d-dam-break-benchmark-paper-n1-a2p25-headspace-200x80-opt5b-vanleer-lowalphaco-sub2-prevcorr-calpha02-exodus'
  [exo]
    type = Exodus
  file_base := '2d-dam-break-benchmark-paper-n1-a2p25-headspace-200x80-opt5b-vanleer-lowalphaco-sub2-prevcorr-calpha02-exodus'
    show = 'pressure alpha vel_x vel_y'
    time_step_interval = 50
  []
[]
