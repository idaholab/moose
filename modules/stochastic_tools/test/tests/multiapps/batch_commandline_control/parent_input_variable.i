[StochasticTools]
[]

[Samplers]
  [sample]
    type = CartesianProduct
    linear_space_items = '1 2 5'
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    input_files = sub_input_variable.i
    sampler = sample
    mode = BATCH-RESET
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = runner
    param_names = 'var1'
    sampler = sample
  []
[]
