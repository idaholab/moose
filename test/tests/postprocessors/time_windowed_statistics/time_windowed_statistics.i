# The monitored postprocessor is f = t, sampled at t = 1, 2, ..., 6 with dt = 1.
# Statistics only use samples with t >= start_time = 2.5, so with window = 3:
#   mean                 : 1, 2, 3, 3.5, 4, 5
#   std                  : 0, 0, 0, sqrt(0.5), 1, 1
# start_time falls inside a time step, so the time integral excludes [2, 2.5] and the sample at
# t = 3 only has weight 0.5:
#   time_integrated_mean : 1, 2, 3, (0.5 * 3 + 4) / 1.5, (0.5 * 3 + 4 + 5) / 2.5,
#                          (0.5 * 3 + 4 + 5 + 6) / 3.5 = 1, 2, 3, 11/3, 4.2, 33/7
#   statistics_time      : 0, 0, 0.5, 1.5, 2.5, 3.5
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
    start_time = 2.5
    window = 3
    statistic = mean
  []
  [std]
    type = TimeWindowedStatisticsPostProcessor
    postprocessor = f
    start_time = 2.5
    window = 3
    statistic = std
  []
  [time_integrated_mean]
    type = TimeWindowedStatisticsPostProcessor
    postprocessor = f
    start_time = 2.5
    window = 3
    statistic = time_integrated_mean
  []
  [statistics_time]
    type = TimeWindowedStatisticsPostProcessor
    postprocessor = f
    start_time = 2.5
    window = 3
    statistic = statistics_time
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
