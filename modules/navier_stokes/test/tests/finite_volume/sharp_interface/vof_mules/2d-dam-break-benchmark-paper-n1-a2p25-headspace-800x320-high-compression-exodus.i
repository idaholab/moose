!include 2d-dam-break-benchmark-paper-n1-a2p25.i

# Long-headspace geometry with the same baseline cell sizes, but refined to
# 800x320 for the mesh-convergence run.
domain_dims_x := ${fparse 10.0 * a_length}
cell_dx := ${fparse domain_dims_x / 800.0}
domain_dims_y := ${fparse 4.0 * dam_y}
cell_dy := ${fparse domain_dims_y / 320.0}
c_alpha := 0.1

[Mesh]
  [mesh]
    ix := '800'
    iy := '320'
  []
[]

[Executioner]
  dt := 3.0e-4
  adjust_momentum_pressure_time_step := true
  momentum_pressure_max_courant := 5.0

  volume_fraction_subcycles := 1
  volume_fraction_max_courant := 0.9
[]

[Outputs]
  console := false
  file_base := '2d-dam-break-benchmark-paper-n1-a2p25-headspace-800x320-high-compression-exodus'
  [exo]
    type = Exodus
    file_base = '2d-dam-break-benchmark-paper-n1-a2p25-headspace-800x320-high-compression-exodus'
    show = 'pressure alpha vel_x vel_y'
    time_step_interval = 50
  []
[]
