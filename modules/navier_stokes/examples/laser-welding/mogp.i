[StochasticTools]
[]

[Distributions]
  [surfacetemp]
    type = Uniform
    lower_bound = 100.0
    upper_bound = 500.0
  []
  [power]
    type = Uniform
    lower_bound = 90.0
    upper_bound = 290.0
  []
[]

[Samplers]
  [sample]
    type = LatinHypercube
    num_rows = 2 # 100
    distributions = 'surfacetemp power'
    execute_on = PRE_MULTIAPP_SETUP
    # min_procs_per_app = 10
    # max_procs_per_app = 10
  []
  [test]
    type = LatinHypercube
    num_rows = 100
    distributions = 'surfacetemp power'
  []
[]

[GlobalParams]
  sampler = sample
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = 2d.i
    mode = batch-reset
    execute_on = initial
    # min_procs_per_app = 10
    # max_procs_per_app = 10
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    param_names = 'surfacetemp power'
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    stochastic_reporter = results
    from_reporter = 'vel_y_vec/vel_y_aux'
  []
[]

[Reporters]
  [results]
    type = StochasticReporter
    outputs = none
  []
  [eval_test]
    type = EvaluateSurrogate
    model = mogp_surrogate
    response_type = vector_real
    parallel_type = ROOT
    execute_on = timestep_end
    sampler = test
  []
  [eval_train]
    type = EvaluateSurrogate
    model = mogp_surrogate
    response_type = vector_real
    parallel_type = ROOT
    execute_on = timestep_end
    sampler = sample
  []
[]

[Trainers]
  [mogp]
    type = MultiOutputGaussianProcessTrainer
    response = results/data:vel_y_vec:vel_y_aux
    response_type = vector_real
    execute_on = initial
    covariance_function = 'covar'
    output_covariance = 'outcovar'
    sampler = sample
    tune_parameters = 'signal_variance length_factor'
    iterations = 20000
    batch_size = 40
    learning_rate = 5e-4
    show_optimization_details = true
  []
[]

[Covariance]
  [covar]
    type= SquaredExponentialCovariance
    signal_variance = 1.0
    noise_variance = 0.0
    length_factor = '1.0 1.0'
  []
[]

[OutputCovariance]
  [outcovar]
    type=IntrinsicCoregionalizationModel
  []
[]

[Surrogates]
  [mogp_surrogate]
    type = MultiOutputGaussianProcess
    trainer = mogp
  []
[]

[VectorPostprocessors]
  [train_data]
    type = SamplerData
    sampler = sample
    execute_on = 'timestep_end'
  []
  [data]
    type = SamplerData
    sampler = test
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  # csv = true
  [out]
    type = JSON
    execute_on = timestep_end
    vectorpostprocessors_as_reporters = true
  []
  [out1]
    type = CSV
    execute_on = timestep_end
  []
[]
