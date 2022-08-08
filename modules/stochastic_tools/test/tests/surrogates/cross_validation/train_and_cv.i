[StochasticTools]
[]

[Samplers]
  [sample]
    type = CartesianProduct
    linear_space_items = '0 0.1515151515 100'
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
    cv_type = "k_fold"
    cv_splits = 2
    cv_n_trials = 3
  []
[]

[Reporters]
  [cv_scores]
    type = CrossValidationScores
    models = 'surrogate'
    execute_on = FINAL
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
  []
[]
