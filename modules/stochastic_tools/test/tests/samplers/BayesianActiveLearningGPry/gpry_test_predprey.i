[StochasticTools]
[]

[Distributions]
  [a]
    # type = TruncatedNormal
    # mean = 0.8
    # standard_deviation = 0.2
    type = Uniform
    lower_bound = 0.15
    upper_bound = 0.95
  []
  [b]
    # type = TruncatedNormal
    # mean = 0.5
    # standard_deviation = 0.2
    type = Uniform
    lower_bound = 0.15
    upper_bound = 0.65
  []
  [c]
    # type = TruncatedNormal
    # mean = 0.5
    # standard_deviation = 0.2
    type = Uniform
    lower_bound = 0.3
    upper_bound = 0.6
  []
  [d]
    # type = TruncatedNormal
    # mean = 0.25
    # standard_deviation = 0.2
    type = Uniform
    lower_bound = 0.05
    upper_bound = 0.35
  []
  [prior_variance]
    type = Uniform
    lower_bound = 0.0
    upper_bound = 0.25
  []
[]

[Samplers]
  [sample]
    type = AffineInvariantDESGPry
    prior_distributions = 'a b c d'
    num_parallel_proposals = 50
    seed = 100
    initial_values = '0.5 0.35 0.5 0.25'
    previous_state = 'mcmc_reporter/inputs'
    previous_state_var = 'mcmc_reporter/variance'
    lower_bound = '0.15 0.15 0.3 0.05'
    upper_bound = '0.95 0.65 0.6 0.35'
    variance_bound = 0.25
    prior_variance = 'prior_variance'
  []
[]

[Reporters]
  [mcmc_reporter]
    type = AIDESGPryTestTransform
    sampler = sample
    gp_evaluator = GP_eval
  []
[]

[Surrogates]
  [GP_eval]
    type = GaussianProcess
    filename = 'gpry_train_predprey_out2_EIGF_mse_new_GP_al_trainer.rd'
  []
[]

[Executioner]
  type = Transient
  num_steps = 500
[]

[Outputs]
  execute_on = TIMESTEP_END
  file_base = 'des_gpry_predprey_test_transform'
  perf_graph = true
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]
