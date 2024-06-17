[StochasticTools]
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = -7.0
    upper_bound = 7.0
  []
[]

[Samplers]
  [sample]
    type = GPryTest
    distributions = 'uniform uniform uniform uniform uniform'
    sorted_indices = 'conditional/sorted_indices'
    num_parallel_proposals = 50
    num_tries = 1000
    seed = 2547
  []
[]

[VectorPostprocessors]
  [values]
    type = nDRosenbrock
    sampler = sample
    execute_on = INITIAL
  []
[]

[Reporters]
  [conditional]
    type = GPryTestLearner
    output_value = values/g_values
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
    tune_parameters = 'signal_variance noise_variance length_factor'
    tuning_algorithm = 'adam'
    iter_adam = 5000
    learning_rate_adam = 0.001 # 0.0001 # 
    show_optimization_details = true
    batch_size = 350
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
    type = SquaredExponentialCovariance
    signal_variance = 4.0
    noise_variance = 0.01
    length_factor = '4.0 4.0 4.0 4.0 4.0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 50
[]

[Outputs]
  csv = false
  [out1_Rosen]
    type = JSON
    execute_system_information_on = NONE
  []
  [out2_Rosen]
    type = SurrogateTrainerOutput
    trainers = 'GP_al_trainer'
    # execute_on = FINAL
  []
[]
