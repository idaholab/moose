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
    force_preic = true
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]
