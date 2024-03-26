[Mesh/generate]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]
[Reporters]
  [sub_rep]
    type = TestDeclareReporter
    distributed_vector_name = dis_vec
  []
[]

[Postprocessors]
  [sub]
    type = ConstantPostprocessor
  []
[]
[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = NONE
    execute_on = TIMESTEP_END
  []
[]
