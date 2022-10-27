[StochasticTools]
[]

[Distributions]
  [studentt]
    type = StudentT
    dof = 17
  []
[]

[Postprocessors]
  [cdf1]
    type = TestDistributionPostprocessor
    distribution = studentt
    value = 0.39548722930287195
    method = cdf
    execute_on = initial
  []
  [cdf2]
    type = TestDistributionPostprocessor
    distribution = studentt
    value = -3.323540778359423
    method = cdf
    execute_on = initial
  []
  [pdf1]
    type = TestDistributionPostprocessor
    distribution = studentt
    value = 0.39548722930287195
    method = pdf
    execute_on = initial
  []
  [pdf2]
    type = TestDistributionPostprocessor
    distribution = studentt
    value = -3.323540778359423
    method = pdf
    execute_on = initial
  []
  [quantile1]
    type = TestDistributionPostprocessor
    distribution = studentt
    value = 0.17746982321896032
    method = quantile
    execute_on = initial
  []
  [quantile2]
    type = TestDistributionPostprocessor
    distribution = studentt
    value = 0.897910497498433
    method = quantile
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
