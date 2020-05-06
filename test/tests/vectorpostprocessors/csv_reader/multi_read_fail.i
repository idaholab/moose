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
  [./reader]
    type = CSVReader
    csv_file = 'example.csv'

    # execute twice to invoke the error
    execute_on = 'TIMESTEP_BEGIN TIMESTEP_END'
  [../]
[]
