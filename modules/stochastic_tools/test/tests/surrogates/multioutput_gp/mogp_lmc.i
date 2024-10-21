[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Normal
    mean = 15.0
    standard_deviation = 2.0
  []
  [bc_dist]
    type = Normal
    mean = 1000.0
    standard_deviation = 100.0
  []
[]

[Samplers]
  [train]
    type = LatinHypercube
    num_rows = 10
    distributions = 'k_dist bc_dist'
    execute_on = PRE_MULTIAPP_SETUP
    seed = 100
  []
  [test]
    type = LatinHypercube
    num_rows = 5
    distributions = 'k_dist bc_dist'
    seed = 101
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    mode = batch-reset
    sampler = train
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = train
    param_names = 'Materials/conductivity/prop_values BCs/right/value'
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    sampler = train
    stochastic_reporter = results
    from_reporter = 'T_vec/T'
  []
[]

[Reporters]
  [results]
    type = StochasticReporter
    outputs = none
  []
  [eval_test]
    type = EvaluateSurrogate
    model = mogp
    response_type = vector_real
    parallel_type = ROOT
    execute_on = timestep_end
    sampler = test
    evaluate_std = true
  []
[]

[Trainers]
  [mogp_trainer]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    covariance_function = 'lmc'
    standardize_params = 'true'
    standardize_data = 'true'
    sampler = train
    response_type = vector_real
    response = results/data:T_vec:T
  []
[]

[Covariance]
  [covar]
    type = SquaredExponentialCovariance
    signal_variance = 2.76658083
    noise_variance = 0.0
    length_factor = '3.67866381 2.63421705'
  []
  [lmc]
    type = LMC
    covariance_functions = covar
    num_outputs = 2
    num_latent_funcs = 1
  []
[]

[Surrogates]
  [mogp]
    type = GaussianProcessSurrogate
    trainer = mogp_trainer
  []
[]

[VectorPostprocessors]
  [train_params]
    type = SamplerData
    sampler = train
    execute_on = final
  []
  [test_params]
    type = SamplerData
    sampler = test
    execute_on = final
  []
  [hyperparams]
    type = GaussianProcessData
    gp_name = mogp
    execute_on = final
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = final
    vectorpostprocessors_as_reporters = true
    execute_system_information_on = NONE
  []
[]
