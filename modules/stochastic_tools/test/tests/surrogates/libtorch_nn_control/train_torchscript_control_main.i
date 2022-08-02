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
    input_files = 'train_torchscript_control_sub.i'
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
    to_reporters = 'results/T_max results/T_min results/control_value'
    from_reporters = 'T_reporter/T_max:value T_reporter/T_min:value T_reporter/control_value:value'
  []
[]

[Trainers]
  [nn_trainer]
    type = LibtorchNeuralNetControlTrainer
    sampler = dummy
    response_reporter = 'results/T_max results/T_min'
    response_constraints ='T_max_constraint T_min_constraint'
    response_shift_coeffs = '0.25 -0.06'
    response_normalization_coeffs = '0.25 0.012'
    control_reporter = 'results/control_value'

    # Parameters for the emulator neural net
    num_emulator_epochs = 1500
    num_emulator_batches = 2
    num_emulator_neurons_per_layer = '48 24'
    emulator_learning_rate = 0.0001

    # Parameters for the control neural net
    num_control_neurons_per_layer = '10 5'
    control_learning_rate = 0.0005
    num_control_epochs = 100
    num_control_loops = 1

    # General data
    filename = mynet.pt
    use_old_response = true
  []
[]

[Reporters]
  [results]
    type = ConstantReporter
    real_vector_names = 'T_max T_min control_value'
    real_vector_values = '0; 0; 0;'
    outputs = csv
    execute_on = timestep_begin
  []
[]

[Functions]
  # These are the constraints, t is T_max/T_min here (time hack)
  [T_max_constraint]
    type = ParsedFunction
    value = 'if(t > 0.1, 0.1, t)'
    # This constraint will penalize maximum temperatures above 0.2
  []
  [T_min_constraint]
    type = ParsedFunction
    value = 'if(t < 0.0, 0.0, t)'
    # This constraint will penalize minimum temperatures below 0
  []
[]

[Executioner]
  type = Transient
  num_steps = 200 # Number of training iterations
[]

[Outputs]
   csv = true
[]
