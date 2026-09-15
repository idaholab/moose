# Verification of simultaneous porous LPS creep laws with distinct stress exponents.

!include creep.i

porosity_name = porosity

[Materials]
  inactive = 'creep'
  [stress]
    inelastic_models := lps
  []

  [lps]
    type = ${AD}PorousViscoplasticityStressUpdate
    coefficient = 'coef_linear coef_cubic coef_45'
    power = '1 3 4.5'
    outputs = all
    porosity_name = ${porosity_name}
    relative_tolerance = 1e-12
    absolute_tolerance = 1e-12
  []
  [coef_linear]
    type = ${AD}ParsedMaterial
    property_name = coef_linear
    expression = '1e-10'
  []
  [coef_cubic]
    type = ${AD}ParsedMaterial
    property_name = coef_cubic
    expression = '1e-20'
  []
  [coef_45]
    type = ${AD}ParsedMaterial
    property_name = coef_45
    expression = '3e-28'
  []
[]

[Postprocessors]
  [eff_creep_strain]
    variable := effective_viscoplasticity
  []
[]
