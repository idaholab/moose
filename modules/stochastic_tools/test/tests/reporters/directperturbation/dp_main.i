[StochasticTools]
[]

[Samplers]
  [directperturbation]
    type = DirectPerturbationSampler
    nominal_parameter_values = '0.1 0.2 0.3'
    relative_perturbation_intervals = '0.1 0.2 0.3'
    perturbation_method = central_difference
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    mode = batch-reset
    sampler = directperturbation
  []
[]

[Controls]
  [param_control]
    type = MultiAppSamplerControl
    multi_app = runner
    param_names = 'x y z'
    sampler = directperturbation
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = runner
    from_reporter = 'const/f1 const/f2 const/f3 const/f_combined'
    stochastic_reporter = storage
    sampler = directperturbation
  []
[]

[Reporters]
  [storage]
    type = StochasticReporter
    outputs = NONE
  []
  [sensitivities]
    type = DirectPerturbationReporter
    reporters = 'storage/data:const:f1 storage/data:const:f2 storage/data:const:f3 storage/data:const:f_combined'
    execute_on = FINAL
    sampler = directperturbation
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = NONE
  []
[]
