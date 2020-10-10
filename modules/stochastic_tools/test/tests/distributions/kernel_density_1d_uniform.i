[StochasticTools]
[]

[Distributions]
  [kernel1d_test]
    type = KernelDensity1D
    bandwidth_rule = 'silverman'
    kernel_function = 'uniform'
    file_name = 'kernel_density_1D_input.csv'
  []
[]

[Postprocessors]
  [cdf]
    type = TestDistributionPostprocessor
    distribution = kernel1d_test
    value = 2.65
    method = cdf
    execute_on = initial
  []
  [pdf]
    type = TestDistributionPostprocessor
    distribution = kernel1d_test
    value = 0.91
    method = pdf
    execute_on = initial
  []
  [quantile]
    type = TestDistributionPostprocessor
    distribution = kernel1d_test
    value = 0.5
    method = quantile
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
