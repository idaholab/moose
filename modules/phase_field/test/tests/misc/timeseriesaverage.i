[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./c]
  [../]
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Functions]
  [./F]
    type = ParsedFunction
    value = 't^2+2'
  [../]
[]

[Postprocessors]
  [./function]
    type = FunctionValuePostprocessor
    function = F
  [../]
  [./dt]
    type = TimestepSize
  [../]
  [./t_average]
    type = TimeSeriesAverage
    average = function
    time = 5.0
    execute_on = 'initial timestep_end'
  [../]
  [./n_average]
    type = TimeSeriesAverage
    average = function
    steps = 3
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  end_time = 15
  [./TimeStepper]
    type = TimeSequenceStepper
    time_sequence = '1 2 4 5.5 7 10'
  [../]
[]

[Outputs]
  csv = true
[]
