[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Problem]
  solve = false
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]

[Postprocessors]
  [time_app1]
    type = TimePostprocessor
  []
  [time_in_app2]
    type = Receiver
  []
[]
