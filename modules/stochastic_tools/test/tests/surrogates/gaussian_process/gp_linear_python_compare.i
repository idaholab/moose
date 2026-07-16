# =============================================================================
# Gaussian Process regression with the custom LinearCovariance kernel
# (see LinearCovariance.C/.h that were supplied), on the toy function
#
#     f(x) = sin(x) + 2*x ,     x in [0, 5]
#
# 10 training points, 20 testing points (see train_x.csv / train_y.csv /
# test_x.csv -- generated once and shared with the matching GPy script so the
# two can be compared apples-to-apples).
#
# Adapted from your GP_linear.i example. Two differences from that file,
# both because our "physical model" here is just a closed-form toy function
# rather than a heat-conduction sub-app:
#   - Samplers/response use CSVSampler + CSVReader directly on train_x.csv /
#     train_y.csv instead of MonteCarlo + MultiApp + SamplerReporterTransfer.
#   - standardize_params/standardize_data are OFF (not 'true' as in your
#     example) so the hyperparameter values here live directly in raw x,y
#     units and can be compared 1:1 against the plain (non-standardized) GPy
#     script. c is therefore set to the domain midpoint (2.5) rather than 0.
#
# This file trains the SAME kernel TWICE:
#   - "..._initial"   -- covariance left at the user-specified starting
#                        hyperparameters, no optimization (tune_parameters is
#                        not set for this Trainer, exactly like your example
#                        file). This is what "Initial hyperparameters" means
#                        below.
#   - "..._optimized" -- identical starting point, but tune_parameters is set
#                        so GaussianProcessTrainer maximizes the marginal
#                        likelihood ("Optimized hyperparameters"). This
#                        tune_parameters/tuning_min/tuning_max/tuning_algorithm
#                        block is NOT shown in your example file (which only
#                        demonstrates a fixed, untrained GP) -- it follows the
#                        syntax documented in MOOSE's stochastic_tools docs,
#                        so please double-check it against your local build.
#   noise_variance (1e-6) is kept FIXED in both cases, following your
#   example's "numerical stability" framing rather than treating it as a
#   parameter to be learned.
#
# Outputs (all via [Outputs], file_base = gp_linear_out):
#   gp_linear_out_train_x_echo_0000.csv   -> training x (echoed from sampler)
#   gp_linear_out_Y_train_0000.csv        -> training y (echoed from CSVReader)
#   gp_linear_out_test_pred_initial_0000.csv    -> X, gp prediction (untuned)
#   gp_linear_out_test_pred_optimized_0000.csv  -> X, gp prediction (tuned)
#   gp_linear_out_hyperparams_initial_0000.csv    -> Initial hyperparameters
#   gp_linear_out_hyperparams_optimized_0000.csv  -> Optimized hyperparameters
#
# NOTE: exact output file names/suffixes depend on your MOOSE version's CSV
# naming convention -- check the working directory after running; the file
# base ("gp_linear_out") and the object names below will always appear in
# whatever it produces.
# =============================================================================

[StochasticTools]
[]

# -----------------------------------------------------------------------------
# Samplers: feed the fixed (non-random) x locations in from CSV files that are
# shared with the Python/GPy comparison script.
# -----------------------------------------------------------------------------
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

# -----------------------------------------------------------------------------
# Training response (y) values, and an echo of the training x values so both
# get written out as the requested "Training Data (x, y)".
# -----------------------------------------------------------------------------
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
# Covariance kernels
#   K(x, x') = bias_variance + signal_variance * (x - c)(x' - c) + noise_variance * I(self)
#
# Initial hyperparameters (chosen by hand, same values used to initialize the
# GPy comparison script). noise_variance follows the convention in your
# GP_linear.i example (a small, FIXED value for numerical stability -- it is
# not tuned below, matching that file's noise_variance = 1e-6 comment):
#     c              = 2.5   (midpoint of the [0, 5] domain -- your example
#                             uses c = 0 because it also standardizes the
#                             training params to zero mean; standardization is
#                             OFF here, see standardize_params below, so 2.5
#                             is the natural pivot in raw x-space)
#     signal_variance = 1.0
#     bias_variance   = 1.0
#     noise_variance  = 1e-6  (fixed, not tuned)
# -----------------------------------------------------------------------------
[Covariance]
  [linear_initial]
    type = LinearCovariance
    c = '2.5'
    signal_variance = 1.0
    bias_variance = 1.0
    noise_variance = 1e-6
  []
  [linear_optimized]
    type = LinearCovariance
    c = '2.5'
    signal_variance = 1.0
    bias_variance = 1.0
    noise_variance = 1e-6
  []
[]

# -----------------------------------------------------------------------------
# Trainers: one untuned (reports back the initial hyperparameters unchanged),
# one with tune_parameters enabled (reports the optimized hyperparameters).
# -----------------------------------------------------------------------------
[Trainers]
  [train_initial]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    sampler = train_sample
    response = 'Y_train/y'
    covariance_function = 'linear_initial'
    standardize_params = false
    standardize_data = false
  []
  [train_optimized]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    sampler = train_sample
    response = 'Y_train/y'
    covariance_function = 'linear_optimized'
    standardize_params = false
    standardize_data = false

    tune_parameters = 'linear_optimized:signal_variance linear_optimized:bias_variance linear_optimized:c'
    tuning_min      = '1e-6                              1e-6                            -10'
    tuning_max      = '1e3                               1e3                             10'

    tuning_algorithm = 'tao'
    # tao_options = '-tao_bncg_type kd'   # uncomment to match the bounded CG
    #                                       variant used in the MOOSE docs examples
  []
[]

# -----------------------------------------------------------------------------
# Surrogates
# -----------------------------------------------------------------------------
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

# -----------------------------------------------------------------------------
# Test-set predictions (mean + std) for both models, plus the tuned/untuned
# hyperparameter dumps.
# -----------------------------------------------------------------------------
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
  file_base = gp_linear_out
  [out]
    type = CSV
    execute_on = 'INITIAL TIMESTEP_END FINAL'
  []
[]
