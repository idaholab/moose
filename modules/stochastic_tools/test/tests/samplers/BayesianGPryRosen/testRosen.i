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
    type = AffineInvariantDESGPry
    prior_distributions = 'uniform uniform uniform uniform uniform'
    num_parallel_proposals = 50
    seed = 100
    initial_values = '1.3361759 1.35774458 1.50500058 1.08151272 1.39076942'
    previous_state = 'mcmc_reporter/inputs'
    previous_state_var = 'mcmc_reporter/variance'
    lower_bound = '-7.0 -7.0 -7.0 -7.0 -7.0'
    upper_bound = '7.0 7.0 7.0 7.0 7.0'
  []
[]

[Reporters]
  [mcmc_reporter]
    type = AIDESGPryTest
    sampler = sample
    gp_evaluator = GP_eval
  []
[]

[Surrogates]
  [GP_eval]
    type = GaussianProcess
    filename = 'trainRosen_out2_Rosen_GP_al_trainer.rd'
  []
[]

[Executioner]
  type = Transient
  num_steps = 500
[]

[Outputs]
  execute_on = TIMESTEP_END
  file_base = 'des_gpry_Rosen_test'
  perf_graph = true
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]
