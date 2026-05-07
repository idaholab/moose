!include 2d-dam-break-benchmark.i

[Executioner]
  # Use a larger nominal timestep and let the reduced-pressure executioner
  # shrink it as needed so the momentum/pressure face-flux Courant number
  # stays below the requested cap.
  dt := 3.0e-4
  adjust_momentum_pressure_time_step := true
  momentum_pressure_max_courant := 2.0

  # Start from one alpha solve per global step and let the executioner
  # increase subcycles so the transported alpha Courant number per substep
  # stays below 1.
  volume_fraction_subcycles := 1
  volume_fraction_max_courant := 1.0
[]
