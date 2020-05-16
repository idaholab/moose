[StochasticTools]
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 5
    upper_bound = 10
  []
[]

[Postprocessors]
  [cdf]
    type = TestDistributionPostprocessor
    distribution = uniform
    value = 7.5
    method = cdf
    execute_on = initial
  []
  [pdf]
    type = TestDistributionPostprocessor
    distribution = uniform
    value = 7.5
    method = pdf
    execute_on = initial
  []
  [quantile]
    type = TestDistributionPostprocessor
    distribution = uniform
    value = 0.5
    method = quantile
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
