[StochasticTools]
[]

[Distributions]
  [kernel1d_test]
    type = KernelDensity1D
    bandwidth_rule = 'standarddeviation'
    kernel_function = 'gaussian'
    data = '4.38 3.34 2.21 1.49 2.44 2.38 2.09 2.63 3.33 3.93' # Normal random numbers with mean 3 and std 1
  []
[]

[Postprocessors]
  [cdf]
    type = TestDistributionPostprocessor
    distribution = kernel1d_test
    value = 3
    method = cdf
    execute_on = initial
  []
  [pdf]
    type = TestDistributionPostprocessor
    distribution = kernel1d_test
    value = 3
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
