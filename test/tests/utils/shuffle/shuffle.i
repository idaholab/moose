[Mesh/gen]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Problem]
  solve = 0
[]

[Executioner]
  type = Steady
[]

[Reporters/test]
  type = TestShuffle
  test_type = swap
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
  []
[]
