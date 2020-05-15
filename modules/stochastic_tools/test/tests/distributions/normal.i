[StochasticTools]
[]

[Distributions]
  [normal_test]
    type = Normal
    mean = 0
    standard_deviation = 1
  []
[]

[Postprocessors]
  [cdf]
    type = TestDistributionPostprocessor
    distribution = normal_test
    value = 0
    method = cdf
    execute_on = initial
  []
  [pdf]
    type = TestDistributionPostprocessor
    distribution = normal_test
    value = 0
    method = pdf
    execute_on = initial
  []
  [quantile]
    type = TestDistributionPostprocessor
    distribution = normal_test
    value = 0.5
    method = quantile
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
