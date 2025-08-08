[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 5
[]

[Problem]
  type = FEProblem
  solve = false
[]

[UserObjects]
  [ud]
    type = MTUserObject
    scalar = 2
    vector = '9 7 5'
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [ud]
    type = NumDOFs
  []
[]
