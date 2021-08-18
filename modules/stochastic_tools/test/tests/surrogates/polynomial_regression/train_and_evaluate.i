[StochasticTools]
[]

[Samplers]
  [sample]
    type = CartesianProduct
    linear_space_items = '0 1 10'
  []
  [test]
    type = CartesianProduct
    linear_space_items = '0.25 0.5 20'
  []
[]

[VectorPostprocessors]
  [values]
    type = GFunction
    sampler = sample
    q_vector = '0'
    execute_on = INITIAL
    outputs = none
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
    trainer = train
  []
[]

[Trainers]
  [train]
    type = PolynomialRegressionTrainer
    regression_type = "ols"
    sampler = sample
    response = values/g_values
    max_degree = 3
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
