[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    positions = '0 0 0'
    input_files = sub.i
  []
[]

[Controls]
  [test]
    type = TestControl
    test_type = 'execflag_error'
  []
[]
