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
    input_files = 'flow_over_circle_linearfv.i'
    mode= batch-reset
  []
[]

[Transfers]
  [nn_transfer]
    type = LibtorchNeuralNetControlTransfer
    to_multi_app = runner
    trainer_name = nn_trainer
    control_name = src_control
    execute_on = TIMESTEP_BEGIN
  []
  [r_transfer]
    type = SamplerReporterTransfer
    from_multi_app = runner
    sampler = dummy
    stochastic_reporter = storage
    from_reporter = 'results/p1:value results/p2:value results/p3:value results/p4:value results/p5:value results/reward:value results/Q:value results/log_prob_Q:value'
  []
[]

[Trainers]
  [nn_trainer]
    type = LibtorchDRLControlTrainer
    response = 'storage/r_transfer:results:p1:value storage/r_transfer:results:p2:value storage/r_transfer:results:p3:value storage/r_transfer:results:p4:value storage/r_transfer:results:p5:value'
    control = 'storage/r_transfer:results:Q:value'
    log_probability = 'storage/r_transfer:results:log_prob_Q:value'
    reward = 'storage/r_transfer:results:reward:value'

    num_epochs = 400
    update_frequency = 1
    decay_factor = 0.99

    loss_print_frequency = 10

    critic_learning_rate = 0.0005
    num_critic_neurons_per_layer = '64 32'

    control_learning_rate = 0.0005
    num_control_neurons_per_layer = '64 32'

    # keep consistent with LibtorchNeuralNetControl
    input_timesteps = 1

    response_scaling_factors = '0.4 0.4 0.4 0.4 0.4'
    response_shift_factors = '-0.4 -0.4 -0.4 -0.4 -0.4'
    action_standard_deviations = '0.02'

    standardize_advantage = true

    read_from_file = false
  []
[]

[Reporters]
  [storage]
    type = StochasticReporter
    parallel_type = ROOT
  []
  [reward]
    type = DRLRewardReporter
    drl_trainer_name = nn_trainer
  []
[]

[Executioner]
  type = Transient
  num_steps = 20
[]

[Outputs]
  file_base = output/train_out
  json = true
  time_step_interval = 1
  execute_on = TIMESTEP_END
[]
