[StochasticTools]
[]

[Samplers]
  [dummy]
    type = CartesianProduct
    linear_space_items = '0 0.01 1'
    min_procs_per_row = 30
    max_procs_per_row = 30
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = dummy
    input_files = 'flow_over_circle_linearfv.i'
    mode = batch-reset
    min_procs_per_app = 30
    max_procs_per_app = 30
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
    from_reporter = 'results/p1x:value results/p2x:value results/p3x:value results/p4x:value results/p5x:value '
                    'results/p1y:value results/p2y:value results/p3y:value results/p4y:value results/p5y:value '
                    'results/reward:value results/Q:value results/log_prob_Q:value'
  []
[]

[Trainers]
  [nn_trainer]
    type = LibtorchDRLControlTrainer
    response = 'storage/r_transfer:results:p1x:value storage/r_transfer:results:p2x:value storage/r_transfer:results:p3x:value storage/r_transfer:results:p4x:value storage/r_transfer:results:p5x:value '
               'storage/r_transfer:results:p1y:value storage/r_transfer:results:p2y:value storage/r_transfer:results:p3y:value storage/r_transfer:results:p4y:value storage/r_transfer:results:p5y:value'
    control = 'storage/r_transfer:results:Q:value'
    log_probability = 'storage/r_transfer:results:log_prob_Q:value'
    reward = 'storage/r_transfer:results:reward:value'

    num_epochs = 25
    update_frequency = 10
    decay_factor = 0.995

    loss_print_frequency = 1

    critic_learning_rate = 0.001
    num_critic_neurons_per_layer = '64 64'
    critic_activation_functions = 'relu relu'

    control_learning_rate = 0.001
    num_control_neurons_per_layer = '512 512'
    control_activation_functions = 'tanh tanh'

    # keep consistent with LibtorchNeuralNetControl
    input_timesteps = 1

    # response_scaling_factors = '13.33 15.38 16.66 38.46 15.38 33.33 40 11.76 4.711 15.38'
    # response_shift_factors = '2.055 2.055 1.93 -0.171 1.945 0.449 -0.525 0.029 0.17675 1.945'

    response_shift_factors = '1.98 1.825 2.015 0.03 1.9 0.58 -0.425 0.06 0.12 -0.02'
    response_scaling_factors = '1.47 1.03 2.60 3.45 2.0 1.19 1.6 2.7 1.47 2.08'

    # response_scaling_factors = '1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0'
    # response_shift_factors = '0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0'

    action_standard_deviations = '0.01'

    standardize_advantage = true

    read_from_file = false

    # min_control_value = ${fparse -0.108}
    # max_control_value = ${fparse 0.108}

    min_control_value = ${fparse -0.080}
    max_control_value = ${fparse 0.080}

    batch_size = 1600
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
  num_steps = 500
[]

[Outputs]
  file_base = output/train_out
  json = true
  execute_on = TIMESTEP_END
[]
