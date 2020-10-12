[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 1
  ymax = 2
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./uu]
  [../]
[]

[VectorPostprocessors]
  [./csv_reader_sub]
    type = CSVReader
    csv_file = "gold/master_receive_values_0001.csv"
  [../]

  [./receive_values_sub]
    type = VectorPostprocessorReceiver
    execute_on = 'timestep_begin'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = sub
  csv = true
[]
