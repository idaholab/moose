[StochasticTools]
[]

[GlobalParams]
  sampler = cv_sampler
  response = results/response_data:max:value
  cv_type = "k_fold"
  cv_splits = 5
  cv_n_trials = 100
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
  [L_dist]
    type = Uniform
    lower_bound = 0.01
    upper_bound = 0.05
  []
  [Tinf_dist]
    type = Uniform
    lower_bound = 290
    upper_bound = 310
  []
[]

[Samplers]
  [cv_sampler]
    type = LatinHypercube
    distributions = 'k_dist q_dist L_dist Tinf_dist'
    num_rows = 1000
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [cv_sub]
    type = SamplerFullSolveMultiApp
    input_files = all_sub.i
    mode = batch-reset
  []
[]

[Controls]
  [pr_cmdline]
    type = MultiAppSamplerControl
    multi_app = cv_sub
    param_names = 'Materials/conductivity/prop_values Kernels/source/value Mesh/xmax BCs/right/value'
  []
[]

[Transfers]
  [response_data]
    type = SamplerReporterTransfer
    from_multi_app = cv_sub
    stochastic_reporter = results
    from_reporter = 'max/value'
  []
[]

[Reporters]
  [results]
    type = StochasticReporter
    outputs = none
  []
  [cv_scores]
    type = CrossValidationScores
    models = 'pr_surr pc_surr np_surr gp_surr ann_surr'
    execute_on = FINAL
  []
[]

[Trainers]
  [pr_max]
    type = PolynomialRegressionTrainer
    regression_type = "ols"
    max_degree = 3
    cv_surrogate = "pr_surr"
    execute_on = timestep_end
  []
  [pc_max]
    type = PolynomialChaosTrainer
    order = 3
    distributions = "k_dist q_dist L_dist Tinf_dist"
    cv_surrogate = "pc_surr"
    execute_on = timestep_end
  []
  [np_max]
    type = NearestPointTrainer
    cv_surrogate = "np_surr"
    execute_on = timestep_end
  []
  [gp_max]
    type = GaussianProcessTrainer
    covariance_function = 'rbf'
    standardize_params = 'true'
    standardize_data = 'true'
    cv_surrogate = "gp_surr"
    execute_on = timestep_end
  []
  [ann_max]
    type = LibtorchANNTrainer
    num_epochs = 100
    num_batches = 5
    num_neurons_per_layer = '64'
    learning_rate = 1e-2
    rel_loss_tol = 1e-4
    filename = mynet.pt
    read_from_file = false
    print_epoch_loss = 0
    activation_function = 'relu'
    cv_surrogate = "ann_surr"
    standardize_input = false
    standardize_output = false
  []
[]

[Covariance]
  [rbf]
    type = SquaredExponentialCovariance
    noise_variance = 3.79e-6
    signal_variance = 1 #Use a signal variance of 1 in the kernel
    length_factor = '5.34471 1.41191 5.90721 2.83723' #Select a length factor for each parameter
  []
[]

[Surrogates]
  [pr_surr]
    type = PolynomialRegressionSurrogate
    trainer = pr_max
  []
  [pc_surr]
    type = PolynomialChaos
    trainer = pc_max
  []
  [np_surr]
    type = NearestPointSurrogate
    trainer = np_max
  []
  [gp_surr]
    type = GaussianProcessSurrogate
    trainer = gp_max
  []
  [ann_surr]
    type = LibtorchANNSurrogate
    trainer = ann_max
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
  []
[]
