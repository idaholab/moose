k_fluid = 0.05
dia = 0.02
Gr = 8e5
Pr = 0.7

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[FunctorMaterials]
  [CC_htc]
    type = ADChurchillChuHTCFunctorMaterial
    Pr = ${Pr}
    Gr = ${Gr}
    k_fluid = ${k_fluid}
    diameter = ${dia}
    htc_name = htc
  []
[]

[Postprocessors]
  [htc]
    type = ADElementExtremeFunctorValue
    functor = htc
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
