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
    num_rows = 1000
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[Surrogates]
  [rbpod]
    type = PODReducedBasisSurrogate
    filename = 'trainer_out_pod_rb.rd'
    change_rank = 'psi'
    new_ranks = '40'
  []
[]

[VectorPostprocessors]
  [res]
    type = PODSurrogateTester
    model = rbpod
    sampler = sample
    variable_name = 'psi'
    to_compute = nodal_l2
  []
[]

[Outputs]
  csv = true
[]
