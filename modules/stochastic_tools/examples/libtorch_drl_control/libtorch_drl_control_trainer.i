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
    from_reporter = 'T_reporter/center_temp_tend:value T_reporter/reward:value T_reporter/top_flux:value T_reporter/log_prob_top_flux:value'
  []
[]

[Trainers]
  [nn_trainer]
    type = LibtorchDRLControlTrainer
    observation = 'storage/r_transfer:T_reporter:center_temp_tend:value'
    control = 'storage/r_transfer:T_reporter:top_flux:value'
    log_probability = 'storage/r_transfer:T_reporter:log_prob_top_flux:value'
    reward = 'storage/r_transfer:T_reporter:reward:value'

    num_epochs = 50
    update_frequency = 1
    decay_factor = 0.8
    lambda_factor = 1.0

    loss_print_frequency = 10

    critic_learning_rate = 0.001
    num_critic_neurons_per_layer = '32 16'
    critic_activation_functions = 'relu relu'

    control_learning_rate = 0.001
    num_control_neurons_per_layer = '32 16'
    control_activation_functions = 'relu relu'


    # keep consistent with LibtorchNeuralNetControl
    input_timesteps = 1
    observation_scaling_factors = '0.03'
    observation_shift_factors = '290'
    action_scaling_factors = 20

    standardize_advantage = true

    batch_size = 80

    read_from_file = false

    entropy_coeff = 0.0

    # min_control_value = ${fparse -0.1}
    # max_control_value = ${fparse 0.1}

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
[]

[Executioner]
  type = Transient
  num_steps = 200
[]

[Outputs]
  file_base = output/train_out
  json = true
  time_step_interval = 1
  execute_on = TIMESTEP_END
[]
