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

[VectorPostprocessors/test]
    type = TestDistributedVectorPostprocessor
    parallel_type = replicated
[]

[Outputs]
  [out]
    type = CSV
    execute_on = TIMESTEP_END
  []
[]
