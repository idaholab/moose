[StochasticTools]
[]

[Samplers]
  [sample]
    type = CartesianProduct
    linear_space_items = '0 0.0125 5
                          0 0.0125 5
                          0 0.0125 5'
  []
[]

[VectorPostprocessors]
  [values]
    type = GFunction
    sampler = sample
    q_vector = '0 0 0'
    execute_on = INITIAL
    outputs = none
  []
[]

[Trainers]
  [train]
    type = LibtorchANNTrainer
    sampler = sample
    response = values/g_values
    num_epochs = 40
    num_batches = 10
    num_neurons_per_layer = '64 32'
    learning_rate = 0.001
    nn_filename = mynet.pt
    read_from_file = false
    print_epoch_loss = 10
    activation_function = 'relu relu'
    max_processes = 1
  []
[]

[Outputs]
  [out]
    type = SurrogateTrainerOutput
    trainers = 'train'
    execute_on = FINAL
  []
[]
