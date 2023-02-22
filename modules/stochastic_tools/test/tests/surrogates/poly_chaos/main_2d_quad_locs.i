[StochasticTools]
[]

[Distributions]
  [D_dist]
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
  [grid]
    type = CartesianProduct
    linear_space_items = '2.5 0.5 10  2.5 0.5 10'
  []
  [quadrature]
    type = Quadrature
    distributions = 'D_dist S_dist'
    execute_on = INITIAL
    order = 5
  []
[]

[MultiApps]
  [quad_sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = quadrature
    mode = batch-restore
  []
[]

[Transfers]
  [quad]
    type = SamplerParameterTransfer
    to_multi_app = quad_sub
    sampler = quadrature
    parameters = 'Materials/diffusivity/prop_values Materials/xs/prop_values'
  []
  [data]
    type = SamplerReporterTransfer
    from_multi_app = quad_sub
    sampler = quadrature
    stochastic_reporter = storage
    from_reporter = avg/value
  []
[]

[Reporters]
  [storage]
    type = StochasticReporter
    outputs = none
  []
  [local_sense]
    type = PolynomialChaosReporter
    pc_name = poly_chaos
    local_sensitivity_sampler = grid
    local_sensitivity_points = '3.14159 3.14159 2.7182 3.14159 3.14159 2.7182 2.7182 2.7182'
    execute_on = final
  []
[]

[Surrogates]
  [poly_chaos]
    type = PolynomialChaos
    trainer = poly_chaos
  []
[]

[Trainers]
  [poly_chaos]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 5
    distributions = 'D_dist S_dist'
    sampler = quadrature
    response = storage/data:avg:value
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
  []
[]
