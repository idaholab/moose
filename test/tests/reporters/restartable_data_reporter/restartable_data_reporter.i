[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
  nx = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Reporters/data]
  type = RestartableDataReporter
  include = 'FEProblemBase/*/time*'
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = none
  []
[]
