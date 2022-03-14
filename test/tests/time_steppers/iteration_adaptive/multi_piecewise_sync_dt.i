[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Functions]
  [./temp_spike1]
    type = PiecewiseLinear
    x = '1 3 5'
    y = '1 4 4'
  [../]
  [./temp_spike2]
    type = PiecewiseLinear
    x = '0 2 4'
    y = '1 1 2'
  [../]
  [./temp_spike3]
    type = PiecewiseConstant
    x = '1 6 8'
    y = '1 4 4'
  [../]
  [./temp_spike4]
    type = PiecewiseConstant
    x = '0 7 9'
    y = '1 1 2'
  [../]
[]

[Executioner]
  type = Transient
  end_time = 10
  verbose = true
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 10
    timestep_limiting_function = 'temp_spike1 temp_spike2 temp_spike3 temp_spike4'
    force_step_every_function_point = true
    post_function_sync_dt = .1
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
