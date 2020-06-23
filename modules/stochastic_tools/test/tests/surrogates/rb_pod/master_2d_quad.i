[StochasticTools]
[]

[Distributions]
  [D_dist]
    type = Uniform
    lower_bound = 2.5
    upper_bound = 7.5
  []
  [sig_dist]
    type = Uniform
    lower_bound = 2.5
    upper_bound = 7.5
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 4
    distributions = 'D_dist sig_dist'
    execute_on = 'timestep_begin'
  []
[]

[MultiApps]
  [sub]
    type = PODFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    execute_on = 'timestep_begin final'
    trainer_name = "pod_rb"
  []
[]

[Transfers]
  [quad]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'Materials/diffusivity/prop_values Materials/xs/prop_values'
    to_control = 'stochastic'
    execute_on = 'timestep_begin'
  []
  [data]
    type = SamplerSolutionTransfer
    multi_app = sub
    sampler = sample
    trainer_name = "pod_rb"
    execute_on = 'timestep_begin'
    direction = 'from_multiapp'
  []
  [mode]
    type = SamplerSolutionTransfer
    multi_app = sub
    sampler = sample
    trainer_name = "pod_rb"
    execute_on = 'final'
    direction = 'to_multiapp'
  []
[]

[Trainers]
  [pod_rb]
    type = PODReducedBasisTrainer
    execute_on = 'timestep_begin' # final'
    var_names = 'u'
    tag_names = 'react diff'
  []
[]

[Outputs]
[]
