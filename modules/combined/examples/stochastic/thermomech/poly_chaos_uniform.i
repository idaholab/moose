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

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 100000
    distributions = 'cond_inner cond_outer heat_source alpha_inner alpha_outer ymod_inner ymod_outer prat_inner prat_outer'
    execute_on = INITIAL
  []
[]

[Surrogates]
  [temp_center_inner]
    type = PolynomialChaos
    filename = 'poly_chaos_train_uniform_out_temp_center_inner.rd'
  []
  [temp_center_outer]
    type = PolynomialChaos
    filename = 'poly_chaos_train_uniform_out_temp_center_outer.rd'
  []
  [temp_end_inner]
    type = PolynomialChaos
    filename = 'poly_chaos_train_uniform_out_temp_end_inner.rd'
  []
  [temp_end_outer]
    type = PolynomialChaos
    filename = 'poly_chaos_train_uniform_out_temp_end_outer.rd'
  []
  [dispx_center_inner]
    type = PolynomialChaos
    filename = 'poly_chaos_train_uniform_out_dispx_center_inner.rd'
  []
  [dispx_center_outer]
    type = PolynomialChaos
    filename = 'poly_chaos_train_uniform_out_dispx_center_outer.rd'
  []
  [dispx_end_inner]
    type = PolynomialChaos
    filename = 'poly_chaos_train_uniform_out_dispx_end_inner.rd'
  []
  [dispx_end_outer]
    type = PolynomialChaos
    filename = 'poly_chaos_train_uniform_out_dispx_end_outer.rd'
  []
  [dispz_inner]
    type = PolynomialChaos
    filename = 'poly_chaos_train_uniform_out_dispz_inner.rd'
  []
  [dispz_outer]
    type = PolynomialChaos
    filename = 'poly_chaos_train_uniform_out_dispz_outer.rd'
  []
[]

[Reporters]
  [storage]
    type = EvaluateSurrogate
    sampler = sample
    model = 'temp_center_inner  temp_center_outer  temp_end_inner  temp_end_outer
             dispx_center_inner dispx_center_outer dispx_end_inner dispx_end_outer
             dispz_inner dispz_outer'
    parallel_type = ROOT
  []
  [stats]
    type = PolynomialChaosReporter
    pc_name = 'temp_center_inner  temp_center_outer  temp_end_inner  temp_end_outer
               dispx_center_inner dispx_center_outer dispx_end_inner dispx_end_outer
               dispz_inner dispz_outer'
    statistics = 'mean stddev'
    include_sobol = true
  []
[]

[Outputs]
  [out]
    type = JSON
  []
  execute_on = TIMESTEP_END
[]
