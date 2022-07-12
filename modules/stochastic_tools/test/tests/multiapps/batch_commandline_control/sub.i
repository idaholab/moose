[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [u]
    initial_condition = 1980
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
[]

[Materials]
  [const]
    type = GenericConstantMaterial
    prop_names = 'A B C D'
    prop_values = '1.0 2.0 3.0 4.0'
  []
[]

[Postprocessors]
  [size]
    type = AverageElementSize
    execute_on = 'initial'
  []
  [prop_A]
    type = ElementAverageMaterialProperty
    mat_prop = A
    execute_on = 'initial'
  []
  [prop_B]
    type = ElementAverageMaterialProperty
    mat_prop = B
    execute_on = 'initial'
  []
  [prop_C]
    type = ElementAverageMaterialProperty
    mat_prop = C
    execute_on = 'initial'
  []
  [prop_D]
    type = ElementAverageMaterialProperty
    mat_prop = D
    execute_on = 'initial'
  []
[]
