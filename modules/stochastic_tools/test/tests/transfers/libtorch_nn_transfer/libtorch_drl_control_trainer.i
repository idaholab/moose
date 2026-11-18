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
    type = SamplerReporterTransfer
    from_multi_app = runner
    sampler = dummy
    stochastic_reporter = storage
    from_reporter = 'T_reporter/center_temp_tend:value T_reporter/env_temp:value T_reporter/reward:value T_reporter/left_flux:value T_reporter/log_prob_left_flux:value'
  []
[]

[Trainers]
  [nn_trainer]
    type = LibtorchDRLControlTrainer
    observation = 'storage/r_transfer:T_reporter:center_temp_tend:value storage/r_transfer:T_reporter:env_temp:value'
    control = 'storage/r_transfer:T_reporter:left_flux:value'
    log_probability = 'storage/r_transfer:T_reporter:log_prob_left_flux:value'
    reward = 'storage/r_transfer:T_reporter:reward:value'

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
    observation_scaling_factors = '0.03 0.03'
    observation_shift_factors = '270 270'
    action_scaling_factors = 100

    read_from_file = false
    shift_outputs = false
  []
[]

[Reporters]
  [storage]
    type = StochasticReporter
    parallel_type = ROOT
    outputs = none
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
