# =============================================================================
# Gaussian Process regression with a PRODUCT kernel:
#     K = SquaredExponentialCovariance (RBF)  .*  MaternHalfIntCovariance (p=2, Matern-5/2)
# built with CovarianceCombiner(operation=Product), on the toy function
#
#     f(x) = sin(x) + 2*x ,     x in [0, 5]
#
# Same data and "initial vs. optimized" structure as gp_linear.i.
#
# Noise handling for Product: per CovarianceCombiner.C, sub-kernels used with
# Product must have noise_variance = 0 (a Hadamard product does not give a
# clean additive diagonal noise term). The class docs for CovarianceCombiner
# explicitly recommend the fix used here: wrap the Product in an outer Sum
# with a LinearCovariance whose signal_variance and bias_variance are both 0,
# so it contributes noise_variance * I on the diagonal and nothing else --
# i.e. LinearCovariance is being (re)used purely as a "pure noise" kernel.
#
#     top = Sum( Product(rbf, matern) , noise_only )
# =============================================================================

[StochasticTools]
[]

[Samplers]
  [train_sample]
    type = CSVSampler
    samples_file = 'train_x.csv'
    execute_on = 'initial'
  []
  [test_sample]
    type = CSVSampler
    samples_file = 'test_x.csv'
    execute_on = 'initial'
  []
[]

[VectorPostprocessors]
  [Y_train]
    type = CSVReader
    csv_file = 'train_y.csv'
  []
  [train_x_echo]
    type = SamplerData
    sampler = train_sample
    execute_on = 'initial'
  []
[]

# -----------------------------------------------------------------------------
# Covariance kernels. Initial hyperparameters (same values used in the GPy
# comparison script):
#     rbf:    signal_variance = 1.0, length_factor = 1.0, noise_variance = 0.0
#     matern: signal_variance = 1.0, length_factor = 1.0, noise_variance = 0.0, p = 2 (nu = 5/2)
#     noise_only (pure-noise LinearCovariance): c = 0, signal_variance = 0,
#                bias_variance = 0, noise_variance = 1e-6
# -----------------------------------------------------------------------------
[Covariance]
  [rbf_initial]
    type = SquaredExponentialCovariance
    signal_variance = 1.0
    noise_variance = 0.0
    length_factor = '1.0'
  []
  [matern_initial]
    type = MaternHalfIntCovariance
    signal_variance = 1.0
    noise_variance = 0.0
    length_factor = '1.0'
    p = 2
  []
  [product_initial]
    type = CovarianceCombiner
    covariance_functions = 'rbf_initial matern_initial'
    operation = Product
  []
  [noise_only_initial]
    type = LinearCovariance
    c = '0'
    signal_variance = 0
    bias_variance = 0
    noise_variance = 1e-6
  []
  [top_initial]
    type = CovarianceCombiner
    covariance_functions = 'product_initial noise_only_initial'
    operation = Sum
  []

  [rbf_optimized]
    type = SquaredExponentialCovariance
    signal_variance = 1.0
    noise_variance = 0.0
    length_factor = '1.0'
  []
  [matern_optimized]
    type = MaternHalfIntCovariance
    signal_variance = 1.0
    noise_variance = 0.0
    length_factor = '1.0'
    p = 2
  []
  [product_optimized]
    type = CovarianceCombiner
    covariance_functions = 'rbf_optimized matern_optimized'
    operation = Product
  []
  [noise_only_optimized]
    type = LinearCovariance
    c = '0'
    signal_variance = 0
    bias_variance = 0
    noise_variance = 1e-6
  []
  [top_optimized]
    type = CovarianceCombiner
    covariance_functions = 'product_optimized noise_only_optimized'
    operation = Sum
  []
[]

[Trainers]
  [train_initial]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    sampler = train_sample
    response = 'Y_train/y'
    covariance_function = 'top_initial'
    standardize_params = false
    standardize_data = false
  []
  [train_optimized]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    sampler = train_sample
    response = 'Y_train/y'
    covariance_function = 'top_optimized'
    standardize_params = false
    standardize_data = false

    # Note: noise_only_optimized:noise_variance is intentionally NOT tuned,
    # kept fixed at 1e-6 (numerical-stability noise), matching the convention
    # in your GP_rbf_times_matern_with_noise.i example. signal_variance/
    # bias_variance of noise_only_optimized are also never tuned -- they must
    # stay at 0 to remain a pure noise term.
    tune_parameters = 'rbf_optimized:signal_variance rbf_optimized:length_factor matern_optimized:signal_variance matern_optimized:length_factor'
    tuning_min      = '1e-6                          1e-6                       1e-6                              1e-6'
    tuning_max      = '1e3                           1e3                        1e3                               1e3'

    tuning_algorithm = 'tao'
  []
[]

[Surrogates]
  [gp_initial]
    type = GaussianProcessSurrogate
    trainer = 'train_initial'
  []
  [gp_optimized]
    type = GaussianProcessSurrogate
    trainer = 'train_optimized'
  []
[]

[Reporters]
  [test_pred_initial]
    type = EvaluateSurrogate
    model = gp_initial
    sampler = test_sample
    evaluate_std = 'true'
    parallel_type = ROOT
    execute_on = final
  []
  [test_pred_optimized]
    type = EvaluateSurrogate
    model = gp_optimized
    sampler = test_sample
    evaluate_std = 'true'
    parallel_type = ROOT
    execute_on = final
  []
[]

[VectorPostprocessors]
  [hyperparams_initial]
    type = GaussianProcessData
    gp_name = 'gp_initial'
    execute_on = final
  []
  [hyperparams_optimized]
    type = GaussianProcessData
    gp_name = 'gp_optimized'
    execute_on = final
  []
[]

[Outputs]
  file_base = gp_product_rbf_matern_out
  [out]
    type = CSV
    execute_on = 'INITIAL TIMESTEP_END FINAL'
  []
[]
