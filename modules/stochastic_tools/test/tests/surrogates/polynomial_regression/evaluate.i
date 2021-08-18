[StochasticTools]
[]

[Samplers]
  [test]
    type = CartesianProduct
    linear_space_items = '0.25 1 10
                          0.25 1 10
                          0.25 1 10'
  []
[]

[Reporters]
  [results]
    type = EvaluateSurrogate
    model = surrogate
    sampler = test
    execute_on = final
    parallel_type = ROOT
  []
[]

[Surrogates]
  [surrogate]
    type = PolynomialRegressionSurrogate
    filename = 'train_out_train.rd'
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
