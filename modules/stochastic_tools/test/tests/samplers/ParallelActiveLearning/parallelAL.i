[StochasticTools]
[]

[Distributions]
  [left]
    type = Normal
    mean = 0.0
    standard_deviation = 1.0
  []
  [right]
    type = Normal
    mean = 0.0
    standard_deviation = 1.0
  []
[]

[ParallelAcquisition]
  [expectedimprovement]
    type = ExpectedImprovement
  []
[]

[Samplers]
  [sample]
    type = GenericActiveLearningSampler
    distributions = 'left right'
    sorted_indices = 'conditional/sorted_indices'
    num_parallel_proposals = 5
    num_tries = 1000
    seed = 100
    initial_values = '0.1 0.1'
    execute_on = PRE_MULTIAPP_SETUP
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
    from_reporter = 'average/value'
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
    param_names = 'left_bc right_bc'
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  [conditional]
    type = GenericActiveLearner
    output_value = constant/reporter_transfer:average:value
    sampler = sample
    al_gp = GP_al_trainer
    gp_evaluator = GP_eval
    acquisition = 'expectedimprovement'
  []
[]

[Trainers]
  [GP_al_trainer]
    type = ActiveLearningGaussianProcess
    covariance_function = 'covar'
    standardize_params = 'true'
    standardize_data = 'true'
    tune_parameters = 'covar:signal_variance covar:length_factor'
    num_iters = 500
    learning_rate = 0.01
    # show_every_nth_iteration = 100
    batch_size = 350
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
    noise_variance = 1e-6
    length_factor = '4.0 4.0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
[]

[Outputs]
  file_base = 'al1'
  [out1_parallelAL]
    type = JSON
    execute_system_information_on = NONE
  []
[]
