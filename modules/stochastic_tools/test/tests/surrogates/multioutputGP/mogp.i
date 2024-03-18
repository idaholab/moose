[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Normal
    mean = 15.0
    standard_deviation = 5.0
  []
  [bc_dist]
    type = Normal
    mean = 1000.0
    standard_deviation = 100.0
  []
[]

[Samplers]
  [sample]
    type = LatinHypercube
    num_rows = 20 # 200
    distributions = 'k_dist bc_dist'
    execute_on = PRE_MULTIAPP_SETUP
  []
  [test]
    type = LatinHypercube
    num_rows = 10 # 100
    distributions = 'k_dist bc_dist'
  []
[]

[GlobalParams]
  sampler = sample
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    mode = batch-reset
    execute_on = initial
    # min_procs_per_app = 2
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    param_names = 'Materials/conductivity/prop_values BCs/right/value'
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
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
    response = results/data:T_vec:T
    response_type = vector_real
    execute_on = initial
    covariance_function = 'covar'
    output_covariance = 'outcovar'
    sampler = sample
    tune_parameters = 'signal_variance length_factor'
    iterations = 10000
    batch_size = 20
    learning_rate = 5e-4
    show_optimization_details = true
  []
[]

[Covariance]
  [covar]
    type= SquaredExponentialCovariance
    signal_variance = 2.76658083
    noise_variance = 0.0
    length_factor = '3.67866381 2.63421705'
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
  # [out1]
  #   type = CSV
  #   execute_on = timestep_end
  # []
[]
