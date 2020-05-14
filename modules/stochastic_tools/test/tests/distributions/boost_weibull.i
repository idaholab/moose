[StochasticTools]
[]

[Distributions]
  [weibull]
    type = BoostWeibull
    shape = 5
    scale = 1
  []
[]

[Postprocessors]
  [cdf]
    type = TestDistributionPostprocessor
    distribution = weibull
    value = 1
    method = cdf
    execute_on = initial
  []
  [pdf]
    type = TestDistributionPostprocessor
    distribution = weibull
    value = 1
    method = pdf
    execute_on = initial
  []
  [quantile]
    type = TestDistributionPostprocessor
    distribution = weibull
    value = 0.63212055882856
    method = quantile
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
  file_base = 'weibull_out'
[]
