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
  [component]
    type = VectorPostprocessorComponent
    vectorpostprocessor = reader
    vector_name = data
    index = 2
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
[]
