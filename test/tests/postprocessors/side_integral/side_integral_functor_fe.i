[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]

[FunctorMaterials]
  [test_fmat]
    type = ADParsedFunctorMaterial
    property_name = test_prop
    expression = '10'
  []
[]

[Postprocessors]
  [test_pp]
    type = ADSideIntegralFunctorPostprocessor
    boundary = top
    functor = test_prop
    functor_argument = face # results in error due to no face info in mesh
    execute_on = 'INITIAL'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
