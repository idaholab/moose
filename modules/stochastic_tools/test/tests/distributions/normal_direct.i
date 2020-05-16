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
    type = TestDistributionDirectPostprocessor
    distribution = normal_test
    value = 0
    method = cdf
    execute_on = initial
  []
  [pdf]
    type = TestDistributionDirectPostprocessor
    distribution = normal_test
    value = 0
    method = pdf
    execute_on = initial
  []
  [quantile]
    type = TestDistributionDirectPostprocessor
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
