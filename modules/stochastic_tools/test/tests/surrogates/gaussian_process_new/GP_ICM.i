[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Uniform
    lower_bound = 1
    upper_bound = 10
  []
  [q_dist]
    type = Uniform
    lower_bound = 9000
    upper_bound = 11000
  []
[]

[Samplers]
  [train_sample]
    type = MonteCarlo
    num_rows = 5
    distributions = 'k_dist q_dist'
    execute_on = PRE_MULTIAPP_SETUP
  []
  [test_sample]
    type = MonteCarlo
    num_rows = 100
    distributions = 'k_dist q_dist'
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = train_sample
    mode = batch-reset
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    param_names = 'Materials/conductivity/prop_values Kernels/source/value'
    sampler = train_sample
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    stochastic_reporter = results
    from_reporter = 'T_vec/T'
    sampler = train_sample
  []
[]


[Reporters]
  [results]
    type = StochasticReporter
    outputs = none
  []
  [eval_test]
    type = EvaluateSurrogate
    model = surr
    response_type = vector_real
    execute_on = timestep_end
    sampler = test_sample
  []
[]

[VectorPostprocessors]
  [hyperparams]
    type = GaussianProcessDataGeneral
    gp_name = 'surr'
    execute_on = final
  []
[]

[Trainers]
  [trainer]
    type = GaussianProcessTrainerGeneral
    execute_on = timestep_end
    covariance_function = 'lmc' #Choose a squared exponential for the kernel
    standardize_params = 'true' #Center and scale the training params
    standardize_data = 'true' #Center and scale the training data
    sampler = train_sample
    response_type = vector_real
    response = results/data:T_vec:T
  []
[]

[Surrogates]
  [surr]
    type = GaussianProcessSurrogateGeneral
    trainer = trainer
  []
[]

[Covariance]
  [qovar]
    type = SquaredExponentialCovariance
    signal_variance = 1 #Use a signal variance of 1 in the kernel
    noise_variance = 1e-6 #A small amount of noise can help with numerical stability
    length_factor = '0.38971 0.38971' #Select a length factor for each parameter (k and q)
  []
  [lmc]
    type = LMC
    covariance_functions = qovar
    num_outputs = 2
    num_latent_funcs = 1
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
[]
