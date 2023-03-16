[StochasticTools]
[]

[Distributions]
  [D012_dist]
    type = Uniform
    lower_bound = 0.2
    upper_bound = 0.8
  []
  [D1_dist]
    type = Uniform
    lower_bound = 0.2
    upper_bound = 0.8
  []
  [D2_dist]
    type = Uniform
    lower_bound = 0.2
    upper_bound = 0.8
  []
  [D3_dist]
    type = Uniform
    lower_bound = 0.15
    upper_bound = 0.6
  []
  [absxs0_dist]
    type = Uniform
    lower_bound = 0.0425
    upper_bound = 0.17
  []
  [absxs1_dist]
    type = Uniform
    lower_bound = 0.065
    upper_bound = 0.26
  []
  [absxs2_dist]
    type = Uniform
    lower_bound = 0.04
    upper_bound = 0.16
  []
  [absxs3_dist]
    type = Uniform
    lower_bound = 0.005
    upper_bound = 0.02
  []
  [src_dist]
    type = Uniform
    lower_bound = 5
    upper_bound = 20
  []
[]

[Samplers]
  [sample]
    type = LatinHypercube
    distributions = 'D012_dist D012_dist D012_dist D3_dist
                     absxs0_dist absxs1_dist absxs2_dist absxs3_dist
                     src_dist src_dist src_dist'
    num_rows = 100
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
    parameters = 'Materials/D0/prop_values
                  Materials/D1/prop_values
                  Materials/D2/prop_values
                  Materials/D3/prop_values
                  Materials/absxs0/prop_values
                  Materials/absxs1/prop_values
                  Materials/absxs2/prop_values
                  Materials/absxs3/prop_values
                  Kernels/src0/value
                  Kernels/src1/value
                  Kernels/src2/value'
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
    trainer_name = 'pod_rb'
    execute_on = 'final'
    check_multiapp_execute_on = false
  []
[]

[Trainers]
  [pod_rb]
    type = PODReducedBasisTrainer
    var_names = 'psi'
    error_res = '1e-9'
    tag_names = 'diff0 diff1 diff2 diff3 abs0 abs1 abs2 abs3 src0 src1 src2'
    tag_types = 'op op op op op op op op src src src'
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
