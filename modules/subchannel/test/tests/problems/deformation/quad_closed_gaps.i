# A pin diameter equal to the pitch closes every gap between two pins, while the gaps between the
# pins and the duct stay open. The center subchannel is bounded only by pin-to-pin gaps, so it
# exchanges no crossflow.

!include quad_pin_diameter.i

[ICs]
  [Dpin_ic]
    value := 0.014605
  []
[]

[Postprocessors]
  [SumWij_center]
    type = SubChannelPointValue
    variable = SumWij
    index = 4
    height = 0.25
    execute_on = 'timestep_end'
  []
[]
