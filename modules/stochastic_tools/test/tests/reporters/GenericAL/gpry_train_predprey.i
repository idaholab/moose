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
  [sample]
    type = GenericActiveLearningSampler
    distributions = 'k_dist q_dist Tinf_dist'
    sorted_indices = 'conditional/sorted_indices'
    num_parallel_proposals = 20
    execute_on = PRE_MULTIAPP_SETUP
    num_tries = 5000
    seed = 100
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
  []
[]

[Transfers]
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'avg/value'
    stochastic_reporter = 'constant'
    from_multi_app = sub
    sampler = sample
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = sample
    param_names = 'a b c'
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  [conditional]
    type = GenericActiveLearner
    output_value = constant/reporter_transfer:avg:value
    sampler = sample
    al_gp = GP_al_trainer
    gp_evaluator = GP_eval
  []
[]

[Trainers]
  [GP_al_trainer]
    type = ActiveLearningGaussianProcess
    covariance_function = 'covar'
    standardize_params = 'true'
    standardize_data = 'true'
    tune_parameters = 'covar:signal_variance covar:noise_variance covar:length_factor'
    num_iters = 10000
    learning_rate = 0.001 # 0.0001 # 
    show_every_nth_iteration = 100
    batch_size = 10000
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
    signal_variance = 4.0
    noise_variance = 0.01
    length_factor = '4.0 4.0 4.0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 30
[]

[Outputs]
  csv = false
  execute_on = TIMESTEP_END
  perf_graph = true
  [out1]  # 
    type = JSON
    execute_system_information_on = NONE
  []
  [out2] # 
    type = SurrogateTrainerOutput
    trainers = 'GP_al_trainer'
    # execute_on = FINAL
  []
[]
