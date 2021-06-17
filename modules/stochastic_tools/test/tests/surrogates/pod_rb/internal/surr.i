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
[]

[Samplers]
  [sample]
    type = LatinHypercube
    distributions = 'k_dist alpha_dist S_dist'
    num_rows = 10
    execute_on = PRE_MULTIAPP_SETUP
    seed = 17
  []
[]

[Surrogates]
  [rbpod]
    type = PODReducedBasisSurrogate
    filename = 'trainer_out_pod_rb.rd'
  []
[]

[VectorPostprocessors]
  [res]
    type = PODSurrogateTester
    model = rbpod
    sampler = sample
    variable_name = "u"
    to_compute = nodal_max
  []
[]

[Outputs]
  csv = true
[]
