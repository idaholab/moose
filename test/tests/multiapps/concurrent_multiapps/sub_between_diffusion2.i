[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
    # partial overlap but also, no equidistant points
    xmin = 0.1111
    ymin = 0.3333
    xmax = 1.211111
    ymax = 1.222222
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
  exodus = true
  execute_on = 'TIMESTEP_END'
[]

[Postprocessors]
  [time_in_app1]
    type = Receiver
  []
  [time_app2]
    type = TimePostprocessor
  []
[]
