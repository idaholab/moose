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

[UserObjects]
  [./tester]
    type = TestCSVReader
    vectorpostprocessor = reader
    vector = year
    gold = '1980 1980 2011 2013'
    rank = 1
  [../]
[]

[VectorPostprocessors]
  [./reader]
    type = CSVReader
    csv_file = example.csv
  [../]
[]

[Outputs]
  csv = true
[]
