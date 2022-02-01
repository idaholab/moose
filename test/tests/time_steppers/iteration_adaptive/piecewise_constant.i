[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Functions]
  [./temp_spike]
    type = PiecewiseConstant
    x = '0 1 1.1 1.2 2'
    y = '1 1 2   1   1'
  [../]
[]

[Executioner]
  type = Transient
  end_time = 2.0
  verbose = true
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.9
    timestep_limiting_function = temp_spike
    force_step_every_function_point = true
  [../]
[]

[Postprocessors]
  [./dt]
    type = TimestepSize
  [../]
[]

[Outputs]
  csv = true
[]
