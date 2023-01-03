[StochasticTools]
[]

[VectorPostprocessors]
  [reader]
    type = CSVReader
    csv_file = data.csv
  []
[]

[Samplers]
  [sample]
    type = CSVSampler
    samples_file = data.csv
    column_indices = '0 1'
    execute_on = INITIAL
  []
[]

[Trainers]
  [train]
    type = PolynomialRegressionTrainer
    regression_type = ols
    sampler = sample
    response = reader/c
    max_degree = 2
    execute_on = INITIAL
  []
[]

[Surrogates]
  [surrogate]
    type = PolynomialRegressionSurrogate
    trainer = train
  []
[]

[AuxVariables]
  [v]
    family = SCALAR
  []

  [T]
    family = SCALAR
    initial_condition = 325
  []
[]

[AuxScalarKernels]
  [scalar_eval]
    type = SurrogateModelScalarAux
    variable = v
    model = 'surrogate'
    parameters = 'T 5'
    execute_on = TIMESTEP_END
  []
[]

[Postprocessors]
  [Tpp]
    type = ScalarVariable
    variable = T
    execute_on = INITIAL
  []
[]

[Outputs]
  csv = true
[]
