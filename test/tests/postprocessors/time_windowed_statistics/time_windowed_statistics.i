# The monitored postprocessor is f = t, sampled at t = 1, 2, ..., 6 with dt = 1.
# Statistics only use samples with t >= start_time = 2, so with window = 3:
#   mean            : 1, 2, 2.5, 3, 4, 5
#   std             : 0, 0, sqrt(0.5), 1, 1, 1
#   cumulative_mean : 1, 2, 2.5, 3, 3.5, 4
#   averaging_time  : 0, 1, 2, 3, 4, 5
[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Functions]
  [f]
    type = ParsedFunction
    expression = 't'
  []
[]

[Postprocessors]
  [f]
    type = FunctionValuePostprocessor
    function = f
  []
  [mean]
    type = TimeWindowedStatisticsPostProcessor
    postprocessor = f
    start_time = 2
    window = 3
    statistic = mean
  []
  [std]
    type = TimeWindowedStatisticsPostProcessor
    postprocessor = f
    start_time = 2
    window = 3
    statistic = std
  []
  [cumulative_mean]
    type = TimeWindowedStatisticsPostProcessor
    postprocessor = f
    start_time = 2
    window = 3
    statistic = cumulative_mean
  []
  [averaging_time]
    type = TimeWindowedStatisticsPostProcessor
    postprocessor = f
    start_time = 2
    window = 3
    statistic = averaging_time
  []
[]

[Executioner]
  type = Transient
  num_steps = 6
  dt = 1
[]

[Outputs]
  csv = true
[]
