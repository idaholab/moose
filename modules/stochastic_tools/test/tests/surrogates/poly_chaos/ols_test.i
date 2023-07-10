[StochasticTools]
[]

[Distributions/uniform]
  type = Uniform
  lower_bound = 0
  upper_bound = 1
[]

[Samplers]
  [csv]
    type = CSVSampler
    samples_file = ols_test.csv
    column_names = 'x_0 x_1 x_2 x_3'
  []
[]

[VectorPostprocessors]
  [results]
    type = CSVReader
    csv_file = ols_test.csv
  []
[]

[GlobalParams]
  distributions = 'uniform uniform uniform uniform'
  order = 3
[]

[Trainers]
  [train_ols]
    type = PolynomialChaosTrainer
    sampler = csv
    response = results/y
    execute_on = TIMESTEP_BEGIN
  []
  [train_pet_ols]
    type = PolynomialChaosTrainer
    sampler = csv
    response = results/y_pet
    execute_on = TIMESTEP_BEGIN
  []
[]

[Surrogates]
  [model_ols]
    type = PolynomialChaos
    trainer = train_ols
  []
  [model_pet_ols]
    type = PolynomialChaos
    trainer = train_pet_ols
  []
[]

[Reporters]
  [stats]
    type = PolynomialChaosReporter
    pc_name = 'model_ols model_pet_ols'
    include_data = true
  []
[]

[Outputs]
  json = true
  execute_on = TIMESTEP_END
[]
