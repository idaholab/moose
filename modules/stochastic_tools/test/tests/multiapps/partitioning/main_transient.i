[StochasticTools]
[]

[Samplers/sample]
  type = CartesianProduct
  linear_space_items = '0 1 5'
  execute_on = PRE_MULTIAPP_SETUP
[]

[GlobalParams]
  sampler = sample
[]

[MultiApps/sub]
  type = SamplerTransientMultiApp
  input_files = sub_transient.i
[]

[Controls/cli]
  type = MultiAppSamplerControl
  multi_app = sub
  param_names = 'Postprocessors/pp1/scale_factor'
[]

[Transfers]
  [param]
    type = SamplerParameterTransfer
    to_multi_app = sub
    parameters = 'Postprocessors/pp2/scale_factor'
  []
  [rep]
    type = SamplerReporterTransfer
    from_multi_app = sub
    stochastic_reporter = reporter
    from_reporter = 'pp1/value'
  []
  [pp]
    type = SamplerPostprocessorTransfer
    from_multi_app = sub
    to_vector_postprocessor = vpp
    from_postprocessor = 'pp2'
  []
[]

[VectorPostprocessors/vpp]
  type = StochasticResults
[]

[Reporters]
  [reporter]
    type = StochasticReporter
    outputs = none
  []
  [check]
    type = TestReporterPartitioning
    sampler = sample
    reporters = 'reporter/rep:pp1:value vpp/pp:pp2'
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
