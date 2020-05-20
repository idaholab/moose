[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables][dummy][][]

[Functions]
  [./interpolate_vpp]
    type = VectorPostprocessorFunction
    vectorpostprocessor_name = read_data
    argument_column = time
    value_column = value
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
[]

[Postprocessors]
  [./check_value]
    type = FunctionValuePostprocessor
    function = interpolate_vpp
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]

[VectorPostprocessors]
  [./read_data]
    type = CSVReader
    csv_file = time_data.csv
    force_preaux = true # necessary so that vpp data exists to interpolate on step 0
    outputs = none
  [../]
[]

[Outputs]
  csv = true # write out FunctionValuePostprocessor results for comparison
[]
