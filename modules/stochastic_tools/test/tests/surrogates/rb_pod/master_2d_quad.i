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
    execute_on = timestep_end
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
  []
[]

[Transfers]
  [quad]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'Materials/diffusivity/prop_values Materials/xs/prop_values'
    to_control = 'stochastic'
  []
  [data]
    type = SamplerSolutionTransfer
    multi_app = sub
    sampler = sample
    trainer_name = "pod_rb"
  []
[]

[Trainers]
  [pod_rb]
    type = PODReducedBasisTrainer
    execute_on = timestep_end
  []
[]

[Outputs]
[]
