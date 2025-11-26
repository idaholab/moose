[StochasticTools]
[]

[Samplers]
  [dummy]
    type = CartesianProduct
    linear_space_items = '0 0.01 1'
    min_procs_per_row = 20
    max_procs_per_row = 20
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = dummy
    input_files = '3.i'
    mode = batch-reset
    min_procs_per_app = 20
    max_procs_per_app = 20
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
    from_reporter = 'results/T1:value results/T2:value results/T3:value results/T4:value '
                    'results/T5:value results/T6:value results/T7:value results/T8:value '
                    'results/reward:value results/speed:value results/log_prob_speed:value'
  []
[]

[Trainers]
  [nn_trainer]
    type = LibtorchDRLControlTrainer
    response = 'storage/r_transfer:results:T1:value storage/r_transfer:results:T2:value storage/r_transfer:results:T3:value storage/r_transfer:results:T4:value '
               'storage/r_transfer:results:T5:value storage/r_transfer:results:T6:value storage/r_transfer:results:T7:value storage/r_transfer:results:T8:value'
    control = 'storage/r_transfer:results:speed:value'
    log_probability = 'storage/r_transfer:results:log_prob_speed:value'
    reward = 'storage/r_transfer:results:reward:value'

    num_epochs = 50
    update_frequency = 1
    decay_factor = 0.99
    lambda_factor = 0.97

    loss_print_frequency = 1

    critic_learning_rate = 0.001
    num_critic_neurons_per_layer = '256 256'
    critic_activation_functions = 'relu relu'

    control_learning_rate = 0.001
    num_control_neurons_per_layer = '256 256'
    control_activation_functions = 'tanh tanh'

    # keep consistent with LibtorchNeuralNetControl
    input_timesteps = 1

    # response_scaling_factors = '13.33 15.38 16.66 38.46 15.38 33.33 40 11.76 4.711 15.38'
    # response_shift_factors = '2.055 2.055 1.93 -0.171 1.945 0.449 -0.525 0.029 0.17675 1.945'

    response_shift_factors = '1500 1500 1500 1500 1500 1500 1500 1500'
    response_scaling_factors = '0.000666667 0.000666667 0.000666667 0.000666667 0.000666667 0.000666667 0.000666667 0.000666667'

    # response_scaling_factors = '1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0'
    # response_shift_factors = '0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0'

    standardize_advantage = true

    read_from_file = false

    # min_control_value = ${fparse -0.108}
    # max_control_value = ${fparse 0.108}

    min_control_value = ${fparse 0.75}
    max_control_value = ${fparse 2.0}

    batch_size = 400
    timestep_window = 10

    entropy_coeff = 0.01
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
  num_steps = 1
[]

[Outputs]
  file_base = output/train_out
  json = true
  execute_on = TIMESTEP_END
[]
