[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Uniform
    lower_bound = 4.0
    upper_bound = 5.0
  []
  [L_dist]
    type = Uniform
    lower_bound = 0.1
    upper_bound = 10.0
  []
  [bc_dist]
    type = Uniform
    lower_bound = 100.0
    upper_bound = 500.0
  []
  [body_dist]
    type = Uniform
    lower_bound = 9000
    upper_bound = 11000
  []
[]

[Samplers]
  [sample]
    type = LatinHypercube
    num_rows = 5
    distributions = 'k_dist L_dist bc_dist body_dist'
    execute_on = PRE_MULTIAPP_SETUP
    # min_procs_per_row = 2
  []
[]

[GlobalParams]
  sampler = sample
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub_vector.i
    mode = batch-reset
    execute_on = initial
    # min_procs_per_app = 2
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    param_names = 'Materials/conductivity/prop_values L BCs/right/value Kernels/source/value'
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    stochastic_reporter = results
    from_reporter = 'T_vec/T T_vec/x'
  []
[]

[Reporters]
  [results]
    type = StochasticReporter
    outputs = none
  []
  # [eval]
  #   type = EvaluateSurrogate
  #   model = pr_surrogate
  #   response_type = vector_real
  #   parallel_type = ROOT
  #   execute_on = timestep_end
  # []
[]

[Trainers]
  [mogp]
    type = MultiOutputGaussianProcessTrainer
    response = results/data:T_vec:T
    response_type = vector_real
    execute_on = initial
    covariance_function = 'covar'
  []
[]

[Covariance]
  [covar]
    type=SquaredExponentialCovariance
    signal_variance = 1
    noise_variance = 1e-3
    length_factor = '1.0 1.0 1.0 1.0'         # There needs to be an error check
  []
[]

# [Surrogates]
#   [pr_surrogate]
#     type = PolynomialRegressionSurrogate
#     trainer = pr
#   []
# []

[Outputs]
  [out]
    type = JSON
    execute_on = timestep_end
  []
[]
