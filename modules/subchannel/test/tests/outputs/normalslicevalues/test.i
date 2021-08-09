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
  [Temp_Out_Point]
    type = QuadSubChannelPointValues
    variable = T
    nx = 1
    ny = 1
    execute_on = final
    file_base = "Temp_out_point"
    height = 3.658
  []
  [Temp_Out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = T
    execute_on = final
    file_base = "Temp_out_matrix"
    height = 3.658
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
