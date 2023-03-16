[StochasticTools]
[]

[Distributions]
  [D012_dist]
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
    num_rows = 1000
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    execute_on = 'timestep_begin'
  []
[]

[Transfers]
  [quad]
    type = SamplerParameterTransfer
    to_multi_app = runner
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
  []
  [results]
    type = SamplerPostprocessorTransfer
    from_multi_app = runner
    sampler = sample
    to_vector_postprocessor = results
    from_postprocessor = 'nodal_l2'
  []
[]

[VectorPostprocessors]
  [results]
    type = StochasticResults
  []
[]

[Outputs]
  csv = true
[]
