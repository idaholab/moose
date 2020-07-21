[StochasticTools]
[]

[Distributions]
  [D0_dist]
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
  [src0_dist]
    type = Uniform
    lower_bound = 5
    upper_bound = 20
  []
  [src1_dist]
    type = Uniform
    lower_bound = 5
    upper_bound = 20
  []
  [src2_dist]
    type = Uniform
    lower_bound = 5
    upper_bound = 20
  []
[]

[Samplers]
  [sample]
    type = LatinHypercube
    distributions = 'D0_dist D1_dist D2_dist D3_dist
                     absxs0_dist absxs1_dist absxs2_dist absxs3_dist
                     src0_dist src1_dist src2_dist'
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
    var_names = 'psi'
    en_limits = '0.999999999'
    tag_names = 'diff0 diff1 diff2 diff3 abs0 abs1 abs2 abs3 src0 src1 src2'
    independent = '0 0 0 0 0 0 0 0 1 1 1'
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
