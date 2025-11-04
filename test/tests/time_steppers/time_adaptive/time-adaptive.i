[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 10
  verbose = true

  [TimeStepper]
    type = SolutionTimeAdaptiveDTTest
    dt = 0.5
    fake_wall_time_sequence = '100 100 200 200 600 300 300 200 200 200 300'
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
  []
[]

[Outputs]
  csv = true
[]
