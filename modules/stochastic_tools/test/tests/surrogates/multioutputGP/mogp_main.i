[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Uniform
    lower_bound = 4.0
    upper_bound = 5.0
  []
  [L_dist]
    type = Uniform
    lower_bound = 2.0
    upper_bound = 10.0
  []
  [bc_dist]
    type = Uniform
    lower_bound = 100.0
    upper_bound = 500.0
  []
  [body_dist]
    type = Uniform
    lower_bound = 50.0
    upper_bound = 100.0
  []
[]

[Samplers]
  [sample]
    type = LatinHypercube
    num_rows = 50 # 200
    distributions = 'k_dist L_dist bc_dist body_dist'
    execute_on = PRE_MULTIAPP_SETUP
  []
  [test]
    type = LatinHypercube
    num_rows = 10 # 1 # 200
    distributions = 'k_dist L_dist bc_dist body_dist'
  []
[]

[GlobalParams]
  sampler = sample
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub_vector.i
    mode = batch-reset
    execute_on = initial
    # min_procs_per_app = 2
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    param_names = 'Materials/conductivity/prop_values L BCs/right/value Kernels/source/value'
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    stochastic_reporter = results
    from_reporter = 'T_vec/T T_vec/x'
  []
[]

[Reporters]
  [results]
    type = StochasticReporter
    outputs = none
  []
  [eval]
    type = EvaluateSurrogate
    model = mogp_surrogate
    response_type = vector_real
    parallel_type = ROOT
    execute_on = final
    sampler = test
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
    iterations = 15000
    batch_size = 50
    learning_rate = 1e-4
    show_optimization_details = true
  []
[]

[Covariance]
  [covar]
    type= MaternHalfIntCovariance # SquaredExponentialCovariance
    p = 1
    signal_variance = 2.76658083
    noise_variance = 1e-8
    length_factor = '3.67866381 2.63421705 1.52975445 3.41603576'         # There needs to be an error check
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

[Outputs]
  [out]
    type = JSON
    execute_on = timestep_end
  []
[]
