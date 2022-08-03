[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[VectorPostprocessors]
  [const]
    type = ConstantVectorPostprocessor
    value = '1 2 3 4 5'
  []
  [distributed]
    type = TestDistributedVectorPostprocessor
    parallel_type = replicated
  []
[]

[Outputs]
  xml = true
[]
