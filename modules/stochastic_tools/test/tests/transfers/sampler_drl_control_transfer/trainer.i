[StochasticTools]
[]

[Samplers]
  [dummy]
    type = CartesianProduct
    linear_space_items = '0 1 4'
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = dummy
    input_files = 'sub.i'
    mode = batch-reset
  []
[]

[Transfers]
  [nn_transfer]
    type = SamplerDRLControlTransfer
    to_multi_app = runner
    trainer_name = nn_trainer
    control_name = src_control
    sampler = dummy
  []
  [r_transfer]
    type = SamplerReporterTransfer
    from_multi_app = runner
    sampler = dummy
    stochastic_reporter = storage
    from_reporter = 'T_reporter/center_temp_tend:value T_reporter/reward:value T_reporter/left_flux:value T_reporter/log_prob_left_flux:value'
  []
[]

[Trainers]
  [nn_trainer]
    type = LibtorchDRLControlTrainer
    observation = 'storage/r_transfer:T_reporter:center_temp_tend:value'
    control = 'storage/r_transfer:T_reporter:left_flux:value'
    log_probability = 'storage/r_transfer:T_reporter:log_prob_left_flux:value'
    reward = 'storage/r_transfer:T_reporter:reward:value'

    num_epochs = 5
    update_frequency = 2
    decay_factor = 0.9
    batch_size = 8

    critic_learning_rate = 0.001
    num_critic_neurons_per_layer = '4'

    control_learning_rate = 0.001
    num_control_neurons_per_layer = '4'

    # Keep consistent with the controller's num_steps_in_period and input_timesteps.
    timestep_window = 2
    input_timesteps = 2
    observation_scaling_factors = '0.03'
    observation_shift_factors = '270'
    action_scaling_factors = 100

    # Bounded controls use the Beta actor.
    min_control_value = 0
    max_control_value = 2

    filename_base = 'sampler_drl'
  []
[]

[Reporters]
  [storage]
    type = StochasticReporter
    parallel_type = ROOT
    outputs = none
  []
  [reward]
    type = DRLRewardReporter
    drl_trainer_name = nn_trainer
  []
  [nn_parameters]
    type = DRLControlNeuralNetParameters
    trainer_name = nn_trainer
  []
[]

[Executioner]
  type = Transient
  num_steps = 4
[]

[Outputs]
  [json_out]
    type = JSON
    execute_system_information_on = NONE
  []
[]
