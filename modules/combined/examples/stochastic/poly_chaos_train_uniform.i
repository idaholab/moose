[StochasticTools]
[]

[Distributions]
  [cond_inner]
    type = Uniform
    lower_bound = 20
    upper_bound = 30
  []
  [cond_outer]
    type = Uniform
    lower_bound = 90
    upper_bound = 110
  []
  [heat_source]
    type = Uniform
    lower_bound = 9000
    upper_bound = 11000
  []
  [alpha_inner]
    type = Uniform
    lower_bound = 1e-6
    upper_bound = 3e-6
  []
  [alpha_outer]
    type = Uniform
    lower_bound = 5e-7
    upper_bound = 1.5e-6
  []
  [ymod_inner]
    type = Uniform
    lower_bound = 2e5
    upper_bound = 2.2e5
  []
  [ymod_outer]
    type = Uniform
    lower_bound = 3e5
    upper_bound = 3.2e5
  []
  [prat_inner]
    type = Uniform
    lower_bound = 0.29
    upper_bound = 0.31
  []
  [prat_outer]
    type = Uniform
    lower_bound = 0.19
    upper_bound = 0.21
  []
[]

[GlobalParams]
  distributions = 'cond_inner cond_outer heat_source alpha_inner alpha_outer ymod_inner ymod_outer prat_inner prat_outer'
[]

[Samplers]
  [sample]
    type = Quadrature
    sparse_grid = smolyak
    order = 5
    execute_on = INITIAL
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = graphite_ring_thermomechanics.i
    sampler = sample
    mode = batch-reset
  []
[]

[Transfers]
  [sub]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'Materials/cond_inner/prop_values Materials/cond_outer/prop_values
                  Postprocessors/heat_source/scale_factor
                  Materials/thermal_strain_inner/thermal_expansion_coeff Materials/thermal_strain_outer/thermal_expansion_coeff
                  Materials/elasticity_tensor_inner/youngs_modulus Materials/elasticity_tensor_outer/youngs_modulus
                  Materials/elasticity_tensor_inner/poissons_ratio Materials/elasticity_tensor_outer/poissons_ratio'
    to_control = 'stochastic'
    check_multiapp_execute_on = false
  []
  [data]
    type = SamplerPostprocessorTransfer
    multi_app = sub
    sampler = sample
    to_vector_postprocessor = storage
    from_postprocessor = 'temp_center_inner  temp_center_outer  temp_end_inner  temp_end_outer
                          dispx_center_inner dispx_center_outer dispx_end_inner dispx_end_outer
                          dispz_inner dispz_outer'
  []
[]

[VectorPostprocessors]
  [storage]
    type = StochasticResults
  []
[]

[Trainers]
  [temp_center_inner]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 4
    sampler = sample
    results_vpp = storage
    results_vector = 'data:temp_center_inner'
  []
  [temp_center_outer]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 4
    sampler = sample
    results_vpp = storage
    results_vector = 'data:temp_center_outer'
  []
  [temp_end_inner]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 4
    sampler = sample
    results_vpp = storage
    results_vector = 'data:temp_end_inner'
  []
  [temp_end_outer]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 4
    sampler = sample
    results_vpp = storage
    results_vector = 'data:temp_end_outer'
  []
  [dispx_center_inner]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 4
    sampler = sample
    results_vpp = storage
    results_vector = 'data:dispx_center_inner'
  []
  [dispx_center_outer]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 4
    sampler = sample
    results_vpp = storage
    results_vector = 'data:dispx_center_outer'
  []
  [dispx_end_inner]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 4
    sampler = sample
    results_vpp = storage
    results_vector = 'data:dispx_end_inner'
  []
  [dispx_end_outer]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 4
    sampler = sample
    results_vpp = storage
    results_vector = 'data:dispx_end_outer'
  []
  [dispz_inner]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 4
    sampler = sample
    results_vpp = storage
    results_vector = 'data:dispz_inner'
  []
  [dispz_outer]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 4
    sampler = sample
    results_vpp = storage
    results_vector = 'data:dispz_outer'
  []
[]

[Outputs]
  [out]
    type = SurrogateTrainerOutput
    trainers = 'temp_center_inner  temp_center_outer  temp_end_inner  temp_end_outer
                dispx_center_inner dispx_center_outer dispx_end_inner dispx_end_outer
                dispz_inner dispz_outer'
    execute_on = FINAL
  []
[]
