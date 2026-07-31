!include c_integral_2d.i

[Materials]
  [numerical_serd]
    type = PowerLawNumericalStrainEnergyRateDensity
    n_exponent = 2
  []
[]

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
