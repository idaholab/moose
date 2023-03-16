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
  [Dir_dist]
    type = Uniform
    lower_bound = 0
    upper_bound = 1
  []
[]

[Samplers]
  [sample]
    type = LatinHypercube
    distributions = 'k_dist alpha_dist S_dist Dir_dist'
    num_rows = 5
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
  [param]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = sample
    parameters = 'Materials/k/prop_values Materials/alpha/prop_values Kernels/source/value BCs/left/value'
    execute_on = 'timestep_begin'
    check_multiapp_execute_on = false
  []
  [data]
    type = PODSamplerSolutionTransfer
    from_multi_app = sub
    sampler = sample
    trainer_name = 'pod_rb'
    execute_on = 'timestep_begin'
    check_multiapp_execute_on = false
  []
  [mode]
    type = PODSamplerSolutionTransfer
    to_multi_app = sub
    sampler = sample
    trainer_name = 'pod_rb'
    execute_on = 'final'
    check_multiapp_execute_on = false
  []
  [res]
    type = PODResidualTransfer
    from_multi_app = sub
    sampler = sample
    trainer_name = "pod_rb"
    execute_on = 'final'
    check_multiapp_execute_on = false
  []
[]

[Trainers]
  [pod_rb]
    type = PODReducedBasisTrainer
    var_names = 'u'
    error_res = '1e-9'
    tag_names = 'diff react bodyf dir_src dir_imp'
    tag_types = 'op op src src_dir op_dir'
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
