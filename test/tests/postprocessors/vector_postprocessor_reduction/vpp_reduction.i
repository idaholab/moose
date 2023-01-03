[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[VectorPostprocessors]
  [reader]
    type = CSVReader
    csv_file = test_data.csv
    outputs = none
  []
[]

[Postprocessors]
  [sum]
    type = VectorPostprocessorReductionValue
    value_type = sum
    vectorpostprocessor = reader
    vector_name = data
    execute_on = 'initial timestep_end'
  []

  [min]
    type = VectorPostprocessorReductionValue
    value_type = min
    vectorpostprocessor = reader
    vector_name = data
    execute_on = 'initial timestep_end'
  []

  [max]
    type = VectorPostprocessorReductionValue
    value_type = max
    vectorpostprocessor = reader
    vector_name = data
    execute_on = 'initial timestep_end'
  []

  [average]
    type = VectorPostprocessorReductionValue
    value_type = average
    vectorpostprocessor = reader
    vector_name = data
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
[]
