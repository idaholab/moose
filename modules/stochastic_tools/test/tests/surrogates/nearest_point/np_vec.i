[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Normal
    mean = 5
    standard_deviation = 2
  []
  [L_dist]
    type = Normal
    mean = 0.03
    standard_deviation = 0.01
  []
[]

[Samplers]
  [sample]
    type = LatinHypercube
    num_rows = 10
    distributions = 'k_dist L_dist'
    execute_on = PRE_MULTIAPP_SETUP
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
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    param_names = 'Materials/conductivity/prop_values L'
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
    model = np_surrogate
    response_type = vector_real
    parallel_type = ROOT
    execute_on = timestep_end
  []
[]

[Trainers]
  [np]
    type = NearestPointTrainer
    response = results/data:T_vec:T
    response_type = vector_real
    execute_on = initial
  []
[]

[Surrogates]
  [np_surrogate]
    type = NearestPointSurrogate
    trainer = np
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = timestep_end
  []
[]
