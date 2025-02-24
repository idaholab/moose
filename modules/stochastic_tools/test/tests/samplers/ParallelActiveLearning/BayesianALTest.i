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

[Samplers]
  [sample]
    type = AffineInvariantDES
    prior_distributions = 'left right'
    num_parallel_proposals = 5
    num_columns = 1
    file_name = 'confg.csv'
    execute_on = PRE_MULTIAPP_SETUP
    seed = 2547
    initial_values = '0.1 0.1'
    previous_state = 'mcmc_reporter/inputs'
    previous_state_var = 'mcmc_reporter/variance'
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  [mcmc_reporter]
    type = GPAffineInvariantDifferentialDecision
    sampler = sample
    gp_evaluator = GP_eval
    likelihoods = ''
  []
[]

[Surrogates]
  [GP_eval]
    type = GaussianProcessSurrogate
    filename = 'b_al_GP_al_trainer.rd'
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
[]

[Outputs]
  file_base = 'b_al_test'
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]
