[StochasticTools]
[]

[Samplers/sample]
  type = CartesianProduct
  linear_space_items = '1 1 3
                        1 1 3'
  execute_on = 'PRE_MULTIAPP_SETUP'
[]

[MultiApps/sub]
  type = SamplerFullSolveMultiApp
  sampler = sample
  input_files = 'sub_steady.i'
  cli_args = 'Mesh/xmax=10;Mesh/ymax=10'
[]

[Transfers]
  inactive = 'param'
  [param]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = sample
    parameters = 'Functions/fun/value Postprocessors/function_val/scale_factor'
  []
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    sampler = sample
    from_reporter = 'size/value function_val/value'
    stochastic_reporter = 'storage'
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = sample
    param_names = 'Functions/fun/value Postprocessors/function_val/scale_factor'
  []
[]

[Reporters/storage]
  type = StochasticReporter
  parallel_type = ROOT
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
