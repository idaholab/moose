[StochasticTools]
[]

[Distributions/uniform]
  type = Uniform
  lower_bound = 0
  upper_bound = 1
[]

[Samplers/sample]
  type = Quadrature
  order = 4
  distributions = 'uniform uniform uniform uniform uniform uniform'
  execute_on = 'initial'
[]

[VectorPostprocessors]
  [results]
    type = GFunction
    sampler = sample
    q_vector = '0 0.5 3 9 99 99'
    execute_on = INITIAL
    outputs = none
  []
[]

[Reporters]
  [sobol]
    type = PolynomialChaosReporter
    pc_name = poly_chaos
    include_sobol = true
    execute_on = timestep_end
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
    order = 4
    distributions = 'uniform uniform uniform uniform uniform uniform'
    sampler = sample
    response = results/g_values
  []
[]

[Outputs]
  execute_on = 'FINAL'
  [out]
    type = JSON
  []
[]
