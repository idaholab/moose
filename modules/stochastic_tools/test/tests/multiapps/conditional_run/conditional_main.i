[StochasticTools]
[]

[Samplers]
  [cart]
    type = CartesianProduct
    linear_space_items = '1 1 3
                          1 1 3'
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = cart
    input_files = 'sub.i'
    mode = batch-reset
    should_run_reporter = conditional/need_sample
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = runner
    sampler = cart
    from_reporter = 'average/value'
    stochastic_reporter = conditional
  []
[]

[Controls]
  [cmdline]
    type = MultiAppCommandLineControl
    multi_app = runner
    sampler = cart
    param_names = 'BCs/left/value BCs/right/value'
  []
[]

[Reporters]
  [conditional]
    type = ConditionalSampleReporter
    sampler = cart
    default_value = 999
    function = 'val1 * val2 >= t'
    sampler_vars = 'val1 val2'
    sampler_var_indices = '0 1'
    parallel_type = ROOT
    execute_on = 'initial timestep_begin'
  []
[]

[Executioner]
  type = Transient
  num_steps = 4
[]

[Outputs]
  execute_on = timestep_end
  [out]
    type = JSON
  []
[]
