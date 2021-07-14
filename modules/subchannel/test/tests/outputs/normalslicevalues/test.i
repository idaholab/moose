T_in = 359.15

[Mesh]
  type = QuadSubChannelMesh
  nx = 3
  ny = 3
  n_cells = 10
  n_blocks = 1
  pitch = 0.25
  rod_diameter = 0.125
  gap = 0.1
  heated_length = 1
  spacer_k = '0.0'
  spacer_z = '0'
[]

[Variables]
  [T]
  []
[]

[ICs]
  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []
[]

[Outputs]
  exodus = true
  [Temp_Out_CSV]
    type = NormalSliceValuesCSV
    variable = T
    execute_on = final
    file_base = "Temp_Out.csv"
    height = 3.658
  []
  [Temp_Out_MATRIX]
    type = NormalSliceValues
    variable = T
    execute_on = final
    file_base = "Temp_Out.txt"
    height = 3.658
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
