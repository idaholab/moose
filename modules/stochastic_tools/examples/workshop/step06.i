[StochasticTools]
[]

[Samplers]
  [csv]
    type = CSVSampler
    samples_file = 'step02_out_sampling_matrix_0001.csv'
    column_names = 'D q T_0 q_0 results:T_avg:value results:q_left:value'
  []
  [resample]
    type = Cartesian1D
    linear_space_items = '0.5 0.1 21
                          50 5 21
                          200 10 21
                          -100 2 26'
    nominal_values = '1 100 300 -100'
  []
[]

[Reporters]
  [sampling_matrix]
    type = StochasticMatrix
    sampler = csv
    sampler_column_names = 'D q T_0 q_0 T_avg q_left'
    outputs = none
  []
  [pr_T_avg_cv]
    type = CrossValidationScores
    models = pr_T_avg
  []
  [evaluate]
    type = EvaluateSurrogate
    model = pr_T_avg
    sampler = resample
    parallel_type = ROOT
  []
[]

[Trainers]
  [poly_reg_T_avg]
    type = PolynomialRegressionTrainer
    sampler = csv
    predictor_cols = '0 1 2 3'
    response = sampling_matrix/T_avg
    max_degree = 3
    regression_type = OLS

    cv_type = K_FOLD
    cv_surrogate = pr_T_avg
  []
[]

[Surrogates]
  [pr_T_avg]
    type = PolynomialRegressionSurrogate
    trainer = poly_reg_T_avg
  []
[]

[Outputs]
  json = true
[]
