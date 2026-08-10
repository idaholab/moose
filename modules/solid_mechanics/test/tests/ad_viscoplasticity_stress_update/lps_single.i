# This test provides an example of an individual LPS viscoplasticity model

!include creep.i

porosity_name = porosity

[Materials]
  inactive = 'creep'
  [stress]
    inelastic_models := lps
  []

  [lps]
    type = ADViscoplasticityStressUpdate
    coefficient = 'coef'
    power = 3
    outputs = all
    porosity_name = ${porosity_name}
    relative_tolerance = 1e-15
  []
  [coef]
    type = ADParsedMaterial
    property_name = coef
    expression = '1e-18 * exp(-4e4 / 1.987 / 1200)'
  []
[]

[Postprocessors]
  [eff_creep_strain]
    variable := effective_viscoplasticity
  []
[]
