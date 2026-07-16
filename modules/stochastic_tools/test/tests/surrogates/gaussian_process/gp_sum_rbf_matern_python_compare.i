# =============================================================================
# Gaussian Process regression with a SUM kernel:
#     K = SquaredExponentialCovariance (RBF)  +  MaternHalfIntCovariance (p=2, i.e. Matern-5/2)
# built with CovarianceCombiner(operation=Sum), on the toy function
#
#     f(x) = sin(x) + 2*x ,     x in [0, 5]
#
# Same data (train_x.csv / train_y.csv / test_x.csv) and same "initial vs.
# optimized" structure as gp_linear.i -- see the comments there for the
# overall workflow explanation.
#
# Noise handling for Sum: per CovarianceCombiner.C, a Sum kernel adds each
# sub-kernel's own noise_variance to the diagonal (sigma_n1^2 + sigma_n2^2).
# To keep this directly comparable to a single GPy "Gaussian_noise" term, only
# the RBF sub-kernel carries a nonzero noise_variance; the Matern sub-kernel's
# noise_variance is fixed at 0.
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
#     rbf:    signal_variance = 1.0, length_factor = 1.0, noise_variance = 1e-6 (fixed, per your example)
#     matern: signal_variance = 1.0, length_factor = 1.0, noise_variance = 0.0, p = 2 (nu = 5/2),
#             matching your GP_rbf_plus_matern.i example
#
# NOTE: double-check against your local SquaredExponentialCovariance /
# MaternHalfIntCovariance source whether "length_factor" is the length-scale
# itself or its reciprocal before comparing numeric values with GPy's
# "lengthscale" -- these two class files weren't among your uploads.
# -----------------------------------------------------------------------------
[Covariance]
  [rbf_initial]
    type = SquaredExponentialCovariance
    signal_variance = 1.0
    noise_variance = 1e-6
    length_factor = '1.0'
  []
  [matern_initial]
    type = MaternHalfIntCovariance
    signal_variance = 1.0
    noise_variance = 0.0
    length_factor = '1.0'
    p = 2
  []
  [sum_initial]
    type = CovarianceCombiner
    covariance_functions = 'rbf_initial matern_initial'
    operation = Sum
  []

  [rbf_optimized]
    type = SquaredExponentialCovariance
    signal_variance = 1.0
    noise_variance = 1e-6
    length_factor = '1.0'
  []
  [matern_optimized]
    type = MaternHalfIntCovariance
    signal_variance = 1.0
    noise_variance = 0.0
    length_factor = '1.0'
    p = 2
  []
  [sum_optimized]
    type = CovarianceCombiner
    covariance_functions = 'rbf_optimized matern_optimized'
    operation = Sum
  []
[]

[Trainers]
  [train_initial]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    sampler = train_sample
    response = 'Y_train/y'
    covariance_function = 'sum_initial'
    standardize_params = false
    standardize_data = false
  []
  [train_optimized]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    sampler = train_sample
    response = 'Y_train/y'
    covariance_function = 'sum_optimized'
    standardize_params = false
    standardize_data = false

    tune_parameters = 'rbf_optimized:signal_variance rbf_optimized:length_factor matern_optimized:signal_variance matern_optimized:length_factor'
    tuning_min      = '1e-6                           1e-6                       1e-6                              1e-6'
    tuning_max      = '1e3                            1e3                        1e3                               1e3'

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
  file_base = gp_sum_rbf_matern_out
  [out]
    type = CSV
    execute_on = 'INITIAL TIMESTEP_END FINAL'
  []
[]
