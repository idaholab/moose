# Verification input for equal-exponent addition and exactly inactive creep mechanisms.

!include creep.i

porosity_name = porosity

[Materials]
  inactive = 'creep'
  [stress]
    inelastic_models := lps
  []

  [lps]
    type = ${AD}PorousViscoplasticityStressUpdate
    coefficient = 'coef_a coef_b'
    power = '3 3'
    outputs = all
    porosity_name = ${porosity_name}
    relative_tolerance = 1e-12
    absolute_tolerance = 1e-12
  []
  [coef_a]
    type = ${AD}ParsedMaterial
    property_name = coef_a
    expression = '4e-20'
  []
  [coef_b]
    type = ${AD}ParsedMaterial
    property_name = coef_b
    expression = '6e-20'
  []
  [coef_sum]
    type = ${AD}ParsedMaterial
    property_name = coef_sum
    expression = '1e-19'
  []
  [coef_linear]
    type = ${AD}ParsedMaterial
    property_name = coef_linear
    expression = '1e-10'
  []
  [coef_zero]
    type = ${AD}ParsedMaterial
    property_name = coef_zero
    expression = '0'
  []
  [coef_negative]
    type = ${AD}ParsedMaterial
    property_name = coef_negative
    expression = '-1e-20'
  []
[]

[Postprocessors]
  [eff_creep_strain]
    variable := effective_viscoplasticity
  []
[]
