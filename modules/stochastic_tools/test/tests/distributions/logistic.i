[StochasticTools]
[]

[Distributions]
  [logistic_test]
    type = Logistic
    location = 1
    shape = 1
  []
[]

[Postprocessors]
  [cdf]
    type = TestDistributionPostprocessor
    distribution = logistic_test
    value = 1.5
    method = cdf
    execute_on = initial
  []
  [pdf]
    type = TestDistributionPostprocessor
    distribution = logistic_test
    value = 1.5
    method = pdf
    execute_on = initial
  []
  [quantile]
    type = TestDistributionPostprocessor
    distribution = logistic_test
    value = 0.5
    method = quantile
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
