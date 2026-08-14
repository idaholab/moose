# This test provides an example of an individual LPS viscoplasticity model

!include creep.i

porosity_name = porosity

[Materials]
  inactive = 'creep'
  [stress]
    inelastic_models := 'lps_one lps_two'
  []

  [lps_one]
    type = ADPorousViscoplasticityStressUpdate
    coefficient = 'coef_one'
    power = 3
    outputs = all
    porosity_name = ${porosity_name}
    relative_tolerance = 1e-11
    base_name = one
    negative_behavior = zero
  []
  [lps_two]
    type = ADPorousViscoplasticityStressUpdate
    coefficient = 'coef_two'
    power = 3
    outputs = all
    porosity_name = ${porosity_name}
    relative_tolerance = 1e-11
    base_name = two
    negative_behavior = zero
  []
  [coef_one]
    type = ADParsedMaterial
    property_name = coef_one
    expression = '9e-19 * exp(-4e4 / 1.987 / 1200)'
  []
  [coef_two]
    type = ADParsedMaterial
    property_name = coef_two
    expression = '1e-19 * exp(-4e4 / 1.987 / 1200)'
  []
  [effective_viscoplasticity]
    type = ADParsedMaterial
    property_name = effective_viscoplasticity
    material_property_names = 'one_effective_viscoplasticity two_effective_viscoplasticity'
    expression = 'one_effective_viscoplasticity + two_effective_viscoplasticity'
    outputs = all
  []
[]

[Postprocessors]
  [eff_creep_strain]
    variable := effective_viscoplasticity
  []
[]
