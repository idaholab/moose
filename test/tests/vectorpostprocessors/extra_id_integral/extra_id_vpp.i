[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'extra_id_vpp.e'
    use_for_exodus_restart = true
    exodus_extra_element_integers = 'pin_id assembly_id'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [value1]
    order = FIRST
    initial_from_file_var = value1
  []
  [value2]
    order = FIRST
    initial_from_file_var = value2
  []
[]

[VectorPostprocessors]
  [integral]
    type = ExtraIDIntegralVectorPostprocessor
    variable = 'value1'
    id_name = 'assembly_id'
  []
[]

[Outputs]
  exodus = false
  csv = true
  execute_on = timestep_end
[]
