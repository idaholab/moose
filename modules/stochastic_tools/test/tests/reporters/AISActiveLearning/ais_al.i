[StochasticTools]
[]

[Distributions]
  [mu1]
    type = Normal
    mean = 0.0
    standard_deviation = 0.5
  []
  [mu2]
    type = Normal
    mean = 1
    standard_deviation = 0.5
  []
[]

[Samplers]
  [sample]
    type = AISActiveLearning
    distributions = 'mu1 mu2'
    proposal_std = '1.0 1.0'
    output_limit = 0.65
    num_samples_train = 15
    num_importance_sampling_steps = 5
    std_factor = 0.9
    initial_values = '-0.103 1.239'
    inputs_reporter = 'adaptive_MC/inputs'
    use_absolute_value = true
    flag_sample = 'conditional/flag_sample'
    seed = 9874
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    mode = batch-reset
    should_run_reporter = conditional/need_sample
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [param]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = sample
    parameters = 'BCs/left/value BCs/right/value'
    to_control = 'stochastic'
  []
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'average/value'
    stochastic_reporter = 'conditional'
    from_multi_app = sub
    sampler = sample
  []
[]

[Reporters]
  [conditional]
    type = ActiveLearningGPDecision
    sampler = sample
    parallel_type = ROOT
    execute_on = 'initial timestep_begin'
    flag_sample = 'flag_sample'
    inputs = 'inputs'
    gp_mean = 'gp_mean'
    gp_std = 'gp_std'
    n_train = 5
    al_gp = GP_al_trainer
    gp_evaluator = GP_eval
    learning_function = 'Ufunction'
    learning_function_parameter = 0.65
    learning_function_threshold = 2.0
  []
  [adaptive_MC]
    type = AdaptiveMonteCarloDecision
    output_value = conditional/gp_mean
    inputs = 'inputs'
    sampler = sample
    gp_decision = conditional
  []
  [ais_stats]
    type = AdaptiveImportanceStats
    output_value = conditional/gp_mean
    sampler = sample
    flag_sample = 'conditional/flag_sample'
  []
[]

[Trainers]
  [GP_al_trainer]
    type = ActiveLearningGaussianProcess
    covariance_function = 'covar'
    standardize_params = 'true'
    standardize_data = 'true'
    tune_parameters = 'covar:signal_variance covar:length_factor'
    num_iters = 2000
    learning_rate = 0.005
  []
[]

[Surrogates]
  [GP_eval]
    type = GaussianProcessSurrogate
    trainer = GP_al_trainer
  []
[]

[Covariance]
  [covar]
    type = SquaredExponentialCovariance
    signal_variance = 1.0
    noise_variance = 1e-8
    length_factor = '1.0 1.0'
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  file_base = 'ais_al'
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]
