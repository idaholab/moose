[StochasticTools]
[]

[Samplers]
  [dummy]
    type = CartesianProduct
    linear_space_items = '0 0.01 1'
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = dummy
    input_files = 'libtorch_drl_control_sub.i'
  []
[]

[Transfers]
  [nn_transfer]
    type = LibtorchNeuralNetControlTransfer
    to_multi_app = runner
    trainer_name = nn_trainer
    control_name = src_control
  []
  [r_transfer]
    type = MultiAppReporterTransfer
    from_multi_app = runner
    to_reporters = 'results/center_temp results/env_temp results/reward results/left_flux '
                   'results/log_prob_left_flux'
    from_reporters = 'T_reporter/center_temp:value T_reporter/env_temp:value T_reporter/reward:value '
                     'T_reporter/left_flux:value T_reporter/log_prob_left_flux:value'
  []
[]

[Trainers]
  [nn_trainer]
    type = LibtorchDRLControlTrainer
    sampler = dummy
    response_reporter = 'results/center_temp results/env_temp'
    control_reporter = 'results/left_flux'
    log_probability_reporter = 'results/log_prob_left_flux'
    reward_reporter = 'results/reward'

    # Parameters for the control neural net
    num_epochs = 1000
    num_batches = 2
    update_frequency = 1
    decay_factor = 0

    critic_learning_rate = 0.0001
    num_critic_neurons_per_layer = '32 10'

    control_learning_rate = 0.0001
    num_control_neurons_per_layer = '32 10'

    # keep consistent with LibtorchNNControl
    input_timesteps = 1
    response_scaling_factors = '0.03 0.03'
    response_shift_factors = '270 270'
    action_standard_deviations = '1 1'

    # General data
    read_from_file = false
  []
[]

[Reporters]
  [results]
    type = ConstantReporter
    real_vector_names = 'center_temp env_temp reward left_flux log_prob_left_flux'
    real_vector_values = '0; 0; 0; 0; 0;'
    outputs = csv
    execute_on = timestep_begin
  []
[]

[Executioner]
  type = Transient
  num_steps = 2 # Number of training iterations
[]

[Outputs]
  file_base = output/train_out
  csv = true
  # execute_on = 'FINAL'
[]
