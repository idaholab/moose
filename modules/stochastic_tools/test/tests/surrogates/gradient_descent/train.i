[StochasticTools]
[]

[VectorPostprocessors]
  [old_faithful]
     type = CSVReader
     csv_file = "../../data/old_faithful.csv"
     execute_on = INITIAL
  []
[]


[UserObjects] #[ObjectiveFunctions]
  [cost]
    type = PolynomialLeastSquares
    vector_postprocessor = old_faithful
    x_vector = "duration"
    y_vector = "wait"
    order = 2
  []
[]


[Trainers]
  [GD]
    type = GradientDescentTrainer
    objective_function = cost
    max_iterations = 10000
    step_size = 0.00001
  []
[]
