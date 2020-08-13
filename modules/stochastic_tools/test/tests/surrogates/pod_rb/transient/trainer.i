[StochasticTools]
  auto_create_executioner = false
[]

[Executioner]
  type = Transient
  num_steps = 2
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
[]

[Samplers]
  [sample]
    type = LatinHypercube
    distributions = 'k_dist alpha_dist S_dist'
    num_rows = 2
    num_bins = 3
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [sub]
    type = PODTransientMultiApp
    input_files = sub.i
    sampler = sample
    trainer_name = 'pod_rb'
  []
[]

[Transfers]
  [param]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'Materials/k/prop_values Materials/alpha/prop_values Kernels/source/value'
    to_control = 'stochastic'
    execute_on = 'initial'
    check_multiapp_execute_on = false
  []
  [snapshots]
    type = PODSamplerSolutionTransfer
    multi_app = sub
    sampler = sample
    trainer_name = 'pod_rb'
    direction = 'from_multiapp'
    execute_on = 'timestep_end'
    check_multiapp_execute_on = false
  []
[]

[Trainers]
  [pod_rb]
    type = PODReducedBasisTrainer
    var_names = 'u'
    error_res = '1e-9'
    tag_names = 'diff react bodyf'
    tag_types = 'op op src'
    execute_on = 'final'
  []
[]
