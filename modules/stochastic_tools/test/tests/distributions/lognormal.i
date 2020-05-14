[StochasticTools]
[]

[Distributions]
  [lognormal_test]
    type = Lognormal
    location = -0.371
    scale = 0.52
  []
[]

[Postprocessors]
  [cdf]
    type = TestDistributionPostprocessor
    distribution = lognormal_test
    value = 0.6
    method = cdf
    execute_on = initial
  []
  [pdf]
    type = TestDistributionPostprocessor
    distribution = lognormal_test
    value = 0.6
    method = pdf
    execute_on = initial
  []
  [quantile]
    type = TestDistributionPostprocessor
    distribution = lognormal_test
    value = 0.5
    method = quantile
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
