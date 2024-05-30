opacity = 1.0
P1_sigma_scattering = 1.0

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[FunctorMaterials]
  [diff_coef_non_AD]
    type = RadiativeP1DiffusionCoefficientMaterial
    opacity = ${opacity}
    sigma_scat_eff = ${P1_sigma_scattering}
    P1_diff_coef_name = 'diff_coef_non_AD'
  []
  [diff_coef_AD]
    type = ADRadiativeP1DiffusionCoefficientMaterial
    opacity = ${opacity}
    sigma_scat_eff = ${P1_sigma_scattering}
    P1_diff_coef_name = 'diff_coef_AD'
  []
[]

[Postprocessors]
  [value_non_AD]
    type = ElementExtremeFunctorValue
    functor = 'diff_coef_non_AD'
    execute_on = 'initial'
  []
  [value_AD]
    type = ADElementExtremeFunctorValue
    functor = 'diff_coef_AD'
    execute_on = 'initial'
  []
  [relative_difference]
    type = RelativeDifferencePostprocessor
    value1 = value_non_AD
    value2 = value_AD
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]
