[StochasticTools]
[]

[Distributions]
  [normal_test]
    type = TruncatedNormal
    mean = 100
    standard_deviation = 25
    lower_bound = 50
    upper_bound = 150
  []
[]

[Postprocessors]
  [cdf]
    type = TestDistributionPostprocessor
    distribution = normal_test
    value = 137.962
    method = cdf
    execute_on = initial
  []
  [pdf]
    type = TestDistributionPostprocessor
    distribution = normal_test
    value = 137.962
    method = pdf
    execute_on = initial
  []
  [quantile]
    type = TestDistributionPostprocessor
    distribution = normal_test
    value = 0.956318
    method = quantile
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
