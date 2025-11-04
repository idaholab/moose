[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 5
[]

[UserObjects]
  [ud]
    type = MTUserObject
    scalar = 2
    vector = '9 7 5'
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
[]

[VectorPostprocessors]
  [ud]
    type = ConstantVectorPostprocessor
    value = 1
  []
[]
