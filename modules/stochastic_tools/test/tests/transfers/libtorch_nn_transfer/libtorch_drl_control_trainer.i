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
    to_reporters = 'results/center_temp results/env_temp results/reward results/left_flux results/log_prob_left_flux'
    from_reporters = 'T_reporter/center_temp_tend:value T_reporter/env_temp:value T_reporter/reward:value T_reporter/left_flux:value T_reporter/log_prob_left_flux:value'
  []
[]

[Trainers]
  [nn_trainer]
    type = LibtorchDRLControlTrainer
    response = 'results/center_temp results/env_temp'
    control = 'results/left_flux'
    log_probability = 'results/log_prob_left_flux'
    reward = 'results/reward'

    num_epochs = 10
    update_frequency = 2
    decay_factor = 0.0

    loss_print_frequency = 3

    critic_learning_rate = 0.0005
    num_critic_neurons_per_layer = '4 2'

    control_learning_rate = 0.0005
    num_control_neurons_per_layer = '4 2'

    # keep consistent with LibtorchNeuralNetControl
    input_timesteps = 2
    response_scaling_factors = '0.03 0.03'
    response_shift_factors = '270 270'
    action_standard_deviations = '0.1'

    read_from_file = false
  []
[]

[Reporters]
  [results]
    type = ConstantReporter
    real_vector_names = 'center_temp env_temp reward left_flux log_prob_left_flux'
    real_vector_values = '0; 0; 0; 0; 0;'
    outputs = 'csv_out'
    execute_on = timestep_begin
  []
  [nn_parameters]
    type = DRLControlNeuralNetParameters
    trainer_name = nn_trainer
    outputs = json_out
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  file_base = train_out
  [json_out]
    type = JSON
    execute_on = TIMESTEP_BEGIN
    execute_system_information_on = NONE
  []
[]
