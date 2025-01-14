[StochasticTools]
[]

[Samplers]
  [dummy]
    type = CartesianProduct
    linear_space_items = '0 0.01 2'
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = dummy
    input_files = 'libtorch_drl_control_sub.i'
    mode = batch-reset
  []
[]

[Transfers]
  [nn_transfer]
    type = SamplerNeuralNetControlTransfer
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
    from_reporter = 'T_reporter/center_temp_tend:value T_reporter/reward:value T_reporter/top_flux:value T_reporter/log_prob_top_flux:value'
  []
[]

[Trainers]
  [nn_trainer]
    type = LibtorchDRLControlTrainer
    response = 'storage/r_transfer:T_reporter:center_temp_tend:value'
    control = 'storage/r_transfer:T_reporter:top_flux:value'
    log_probability = 'storage/r_transfer:T_reporter:log_prob_top_flux:value'
    reward = 'storage/r_transfer:T_reporter:reward:value'

    num_epochs = 400
    update_frequency = 1
    decay_factor = 0.8

    loss_print_frequency = 40

    critic_learning_rate = 0.0005
    num_critic_neurons_per_layer = '32 16'

    control_learning_rate = 0.0005
    num_control_neurons_per_layer = '16 6'

    # keep consistent with LibtorchNeuralNetControl
    input_timesteps = 1
    response_scaling_factors = '0.03'
    response_shift_factors = '290'
    action_standard_deviations = '0.01'

    standardize_advantage = true

    read_from_file = false
  []
[]

[Reporters]
  [storage]
    type = StochasticReporter
    parallel_type = ROOT
  []
  # [reward]
  #   type = DRLRewardReporter
  #   drl_trainer_name = nn_trainer
  # []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  file_base = output/train_out
  json = true
  time_step_interval = 1
  execute_on = TIMESTEP_END
[]
