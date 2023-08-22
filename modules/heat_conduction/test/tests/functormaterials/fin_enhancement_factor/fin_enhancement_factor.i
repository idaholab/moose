[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[FunctorMaterials]
  [fin_fmat]
    type = FinEnhancementFactorFunctorMaterial
    fin_efficiency = 0.6
    fin_area_fraction = 0.95
    area_increase_factor = 5.0
    fin_enhancement_factor_name = factor
  []
[]

[Postprocessors]
  [fin_enhancement_factor]
    type = ElementExtremeFunctorValue
    functor = factor
    execute_on = 'INITIAL'
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
