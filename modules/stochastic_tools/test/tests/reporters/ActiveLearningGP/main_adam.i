[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Uniform
    lower_bound = 5
    upper_bound = 20
  []
  [q_dist]
    type = Uniform
    lower_bound = 7000
    upper_bound = 13000
  []
  [Tinf_dist]
    type = Uniform
    lower_bound = 250
    upper_bound = 350
  []
[]

[Samplers]
  [mc]
    type = ActiveLearningMonteCarloSampler
    num_batch = 1
    distributions = 'k_dist q_dist Tinf_dist'
    flag_sample = 'conditional/flag_sample'
    seed = 5
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    sampler = mc
    input_files = 'sub.i'
    mode = batch-reset
    should_run_reporter = conditional/need_sample
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [sub]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = mc
    parameters = 'Materials/conductivity/prop_values Kernels/source/value BCs/right/value'
    to_control = 'stochastic'
    check_multiapp_execute_on = false
  []
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'avg/value'
    stochastic_reporter = 'conditional'
    from_multi_app = sub
    sampler = mc
  []
[]

[Reporters]
  [conditional]
    type =  ActiveLearningGPDecision
    sampler = mc
    parallel_type = ROOT
    execute_on = 'timestep_begin'
    flag_sample = 'flag_sample'
    inputs = 'inputs'
    gp_mean = 'gp_mean'
    gp_std = 'gp_std'
    n_train = 7
    al_gp = GP_al_trainer
    gp_evaluator = GP_eval
    learning_function='Ufunction'
    learning_function_parameter = 349.345
    learning_function_threshold=2.0
  []
[]

[Trainers]
  [GP_al_trainer]
    type = ActiveLearningGaussianProcess
    covariance_function = 'covar'
    standardize_params = 'true'
    standardize_data = 'true'
    tune_parameters = 'signal_variance length_factor'
    tuning_algorithm = 'adam'
    iter_adam = 1000
    learning_rate_adam = 0.005
    # show_optimization_details = true
  []
[]

[Surrogates]
  [GP_eval]
    type = GaussianProcess
    trainer = GP_al_trainer
  []
[]

[Covariance]
  [covar]
    type= SquaredExponentialCovariance
    signal_variance = 1.0
    noise_variance = 1e-4
    length_factor = '1.0 1.0 1.0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 20
[]

[Outputs]
  file_base = 'single_proc_single_row_ufunction'
  [out]
    type = JSON
    execute_system_information_on = none
  []
[]
