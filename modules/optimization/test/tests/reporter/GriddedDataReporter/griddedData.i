[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [null]
    type = NullKernel
    variable = u
  []
[]

[Reporters]
  [parameter_info]
    type = GriddedDataReporter
    data_file = 'gridded_data.txt'
    outputs = out
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = none
  []
[]
