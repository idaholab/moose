[Problem]
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Executioner]
  type = Transient
  start_time = 0.0
  end_time = 0.0
[]

[VectorPostprocessors]
  [./csv_data]
    type = CSVReader
    csv_file = fit_data_0.csv
    header = true
    outputs = none
  [../]
  [./least_squares_fit_coeffs]
    type = LeastSquaresFit
    vectorpostprocessor = csv_data
    x_name = 'id'
    y_name = 'u'
    order = 0
    output = coefficients
    truncate_order = false
    execute_on = initial
  [../]
[]

[Outputs]
  file_base = csv0
  execute_on = initial
  csv = true
[]
