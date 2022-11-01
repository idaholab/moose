[StochasticTools]
[]

[Distributions]
  [beta]
    type = Beta
    alpha = 1.9591684282393695
    beta = 3.1004701483699475
  []
[]

[Postprocessors]
  [cdf]
    type = TestDistributionPostprocessor
    distribution = beta
    value = 0.5314213417160761
    method = cdf
    execute_on = initial
  []
  [pdf]
    type = TestDistributionPostprocessor
    distribution = beta
    value = 0.9689028504206514
    method = pdf
    execute_on = initial
  []
  [quantile]
    type = TestDistributionPostprocessor
    distribution = beta
    value = 0.49637324510635306
    method = quantile
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
