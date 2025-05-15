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

[Likelihood]
  [gaussian]
    type = Gaussian
    noise = 'noise_specified/noise_specified'
    file_name = 'exp_0_05.csv'
    # log_likelihood = true
  []
[]

[ParallelAcquisition]
  [BayesianPosterior]
    type = BayesianPosteriorTargeted
  []
[]

[Samplers]
  [sample]
    type = BayesianActiveLearningSampler
    prior_distributions = 'left right'
    sorted_indices = 'conditional/sorted_indices'
    num_parallel_proposals = 5
    num_tries = 100
    seed = 200
    file_name = 'confg.csv'
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
    param_names = 'left_bc right_bc mesh1'
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  [noise_specified]
    type = ConstantReporter
    real_names = 'noise_specified'
    real_values = '0.05'
  []
  [conditional]
    type = BayesianActiveLearner
    output_value = constant/reporter_transfer:average:value
    sampler = sample
    al_gp = GP_al_trainer
    gp_evaluator = GP_eval
    acquisition = 'BayesianPosterior'
    likelihoods = 'gaussian'
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
  file_base = 'b_al'
  [out1_parallelAL]
    type = JSON
    execute_system_information_on = NONE
  []
  [out2_BayesianAL]
    type = SurrogateTrainerOutput
    trainers = 'GP_al_trainer'
    execute_on = FINAL
  []
[]
