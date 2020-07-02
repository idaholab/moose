[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Uniform
    lower_bound = 2.5
    upper_bound = 7.5
  []
  [alpha_dist]
    type = Uniform
    lower_bound = 2.5
    upper_bound = 7.5
  []
  [S_dist]
    type = Uniform
    lower_bound = 2.5
    upper_bound = 7.5
  []
  [bc_dist]
    type = Uniform
    lower_bound = 0
    upper_bound = 1
  []
[]

[Samplers]
  [sample]
    type = LatinHypercube
    distributions = 'k_dist alpha_dist S_dist bc_dist'
    num_rows = 100
    num_bins = 10
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [sub]
    type = PODFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    trainer_name = 'pod_rb'
    execute_on = 'timestep_begin final'
  []
[]

[Transfers]
  [quad]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'Materials/k/prop_values Materials/alpha/prop_values Kernels/source/value BCs/left/value'
    to_control = 'stochastic'
    execute_on = 'timestep_begin'
  []
  [data]
    type = SamplerSolutionTransfer
    multi_app = sub
    sampler = sample
    trainer_name = 'pod_rb'
    direction = 'from_multiapp'
    execute_on = 'timestep_begin'
  []
  [mode]
    type = SamplerSolutionTransfer
    multi_app = sub
    sampler = sample
    trainer_name = 'pod_rb'
    direction = 'to_multiapp'
    execute_on = 'final'
  []
  [res]
    type = ResidualTransfer
    multi_app = sub
    sampler = sample
    trainer_name = 'pod_rb'
    execute_on = 'final'
  []
[]

[Trainers]
  [pod_rb]
    type = PODReducedBasisTrainer
    var_names = 'u'
    en_limits = '0.999999999'
    tag_names = 'diff react bodyf dir_bc_src dir_bc_imp'
    dir_tag_names = 'dir_bc_src dir_bc_imp'
    independent = '0 0 1 1 0'
    execute_on = 'timestep_begin final'
  []
[]

[Outputs]
  [out]
    type = SurrogateTrainerOutput
    trainers = 'pod_rb'
    execute_on = FINAL
  []
[]
