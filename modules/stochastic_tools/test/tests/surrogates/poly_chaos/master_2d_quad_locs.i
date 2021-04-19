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
    multi_app = quad_sub
    sampler = quadrature
    parameters = 'Materials/diffusivity/prop_values Materials/xs/prop_values'
    to_control = 'stochastic'
  []
  [data]
    type = SamplerPostprocessorTransfer
    multi_app = quad_sub
    sampler = quadrature
    to_vector_postprocessor = storage
    from_postprocessor = avg
  []
[]

[VectorPostprocessors]
  [storage]
    type = StochasticResults
    parallel_type = REPLICATED
  []
  [local_sense]
    type = PolynomialChaosLocalSensitivity
    pc_name = poly_chaos
    local_points_sampler = grid
    local_points = '3.14159 3.14159 2.7182 3.14159 3.14159 2.7182 2.7182 2.7182'
    output_points = true
    sensitivity_parameters = '0 1'
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
    response = storage/data:avg
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
[]
