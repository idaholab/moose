[StochasticTools]
[]

[Samplers]
  [sample]
    type = CSVSampler
    samples_file = 'test_inp.csv'
    execute_on = 'PRE_MULTIAPP_SETUP'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub_new.i
    mode = batch-reset
    execute_on = initial
    sampler = sample
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    stochastic_reporter = results
    from_reporter = 'T_vec/T'
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    param_names = 'Materials/conductivity/prop_values BCs/right/value'
    sampler = sample
  []
[]

[Reporters]
  [results]
    type = StochasticReporter
    outputs = none
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  # file_base = 'mc_mean_homogenized_dtf'
  # [out]
  #   type = CSV # JSON
  #   # execute_system_information_on = NONE
  # []
[]
