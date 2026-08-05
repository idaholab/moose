!include c_integral_2d.i

[Postprocessors]
  [serd]
    type = ElementAverageMaterialProperty
    mat_prop = strain_energy_rate_density
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
