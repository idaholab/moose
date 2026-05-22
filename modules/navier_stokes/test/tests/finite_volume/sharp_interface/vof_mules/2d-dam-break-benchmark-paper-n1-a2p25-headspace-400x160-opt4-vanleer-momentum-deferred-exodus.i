!include 2d-dam-break-benchmark-paper-n1-a2p25-headspace.i

# 400x160 matched Martin-Moyce comparison case with:
# - Van Leer alpha correction
# - lower alpha Courant cap
# - two minimum alpha subcycles
# - previous-correction reuse
# - momentum deferred correction via Venkatakrishnan interpolation
c_alpha := 0.1

[Physics]
  [NavierStokes]
    [ConservativeSharpInterfaceVOFSegregated]
      [vof]
        alpha_correction_scheme = 'vanLeer'
        alpha_apply_prev_corr := true
      []
    []
    [ConservativeSharpInterfaceFlowSegregated]
      [flow]
        momentum_advection_interpolation := 'venkatakrishnan'
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
  console = true
  file_base := '2d-dam-break-benchmark-paper-n1-a2p25-headspace-400x160-opt4-vanleer-momentum-deferred-exodus'
  [exo]
    type = Exodus
    file_base := '2d-dam-break-benchmark-paper-n1-a2p25-headspace-400x160-opt4-vanleer-momentum-deferred-exodus'
    show = 'pressure alpha rhou rhov'
    time_step_interval = 50
  []
[]
