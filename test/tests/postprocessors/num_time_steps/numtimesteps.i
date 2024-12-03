[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Postprocessors/timestep_ctr]
  type = NumTimeSteps
[]

[Problem]
  solve = False
[]

[Executioner]
  type = Transient
  dt = 0.001
  num_steps = 10
[]

[Outputs]
  csv = true
  exodus = false
[]
