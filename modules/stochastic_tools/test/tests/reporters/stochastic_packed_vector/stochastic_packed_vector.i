[StochasticTools]
[]

[Samplers]
  [csv]
    type = CSVSampler
    samples_file = './sampler_data.csv'
    column_names = 'result_0 result_1 sample_0 sample_1 sample_2 sample_3 sample_4 sample_5 sample_6'
  []
[]


[Reporters]
  [sampling_matrix]
    type = StochasticMatrix
    sampler = csv
    sampler_column_names = 'result_0 result_1 sample_0 sample_1 sample_2 sample_3 sample_4 sample_5 sample_6'
    parallel_type = ROOT
  []
  [result_packing]
    type = StochasticPackedVector
    sampler = csv
    columns = '0 1'
    output_name = 'result_pair'
    parallel_type = ROOT
    execute_on = TIMESTEP_END
  []
[]

[Trainers]
  [gaussian_process]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    covariance_function = 'lmc'
    standardize_params = 'true'
    standardize_data = 'true'
    sampler = csv
    predictor_cols = '2 3 4 5 6 7 8'
    response_type = vector_real
    response = 'result_packing/result_pair'
    tune_parameters = 'lmc:acoeff_0 lmc:lambda_0 rbf:signal_variance rbf:length_factor rbf:noise_variance'
    tuning_min = '1e-9 1e-9 1e-9 1e-9 1e-3'
    tuning_max = '1e16 1e16 1e16 1e16 1e2'
    batch_size = 8
    num_iters = 100
    learning_rate = 5e-3
    show_every_nth_iteration = 10
  []
[]

[Covariance]
  [rbf]
    type = SquaredExponentialCovariance
    signal_variance = 1 #Use a signal variance of 1 in the kernel
    noise_variance = 5e-2 #A small amount of noise can help with numerical stability
    length_factor = '0.38971 0.38971 0.38971 0.38971 0.38971 0.38971 0.38971' #Select a length factor for each parameter (k and q)
  []
  [lmc]
    type = LMC
    covariance_functions = rbf
    num_outputs = 2
    num_latent_funcs = 1
  []
[]

[Outputs]
  json = true
[]
