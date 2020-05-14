[StochasticTools]
[]

[Distributions]
  [johnsonsb_test]
    type = JohnsonSB
    a = 1
    b = 2
    alpha_1 = 1
    alpha_2 = 2
  []
[]

[Postprocessors]
  [cdf]
    type = TestDistributionPostprocessor
    distribution = johnsonsb_test
    value = 1.5
    method = cdf
    execute_on = initial
  []
  [pdf]
    type = TestDistributionPostprocessor
    distribution = johnsonsb_test
    value = 1.5
    method = pdf
    execute_on = initial
  []
  [quantile]
    type = TestDistributionPostprocessor
    distribution = johnsonsb_test
    value = 0.5
    method = quantile
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
