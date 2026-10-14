[StochasticTools]
[]

[Reporters]
  # Two rollout samples with three raw entries each: the initial value and two time steps.
  [data]
    type = ConstantReporter
    real_vector_vector_names = 'observation action log_probability reward'
    real_vector_vector_values = '300 301 302; 300 302 304 | 0 1 2; 0 2 4 | 0 -1 -1; 0 -2 -2 | 0 1 2; 0 2 4'
  []
[]

[Trainers]
  [nn_trainer]
    type = LibtorchDRLControlTrainer
    observation = 'data/observation'
    control = 'data/action'
    log_probability = 'data/log_probability'
    reward = 'data/reward'

    num_epochs = 1
    critic_learning_rate = 0.001
    num_critic_neurons_per_layer = '2'
    control_learning_rate = 0.001
    num_control_neurons_per_layer = '2'

    filename_base = 'trainer_errors'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
