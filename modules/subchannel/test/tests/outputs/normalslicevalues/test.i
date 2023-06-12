[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
    nx = 3
    ny = 3
    n_cells = 10
    pitch = 0.25
    rod_diameter = 0.125
    gap = 0.1
    heated_length = 1
    spacer_k = '0.0'
    spacer_z = '0'
  []
[]

[Functions]
  [T_fn]
    type = ParsedFunction
    expression = if(z>0.5,100.0,50.0)
  []
[]

[Variables]
  [T]
  []
[]

[ICs]
  [T_ic]
    type = FunctionIC
    variable = T
    function = T_fn
  []
[]

[Outputs]
  exodus = false
  [Temp_Out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = T
    execute_on = final
    file_base = "Temp_out_matrix.csv"
    height = 0.55
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
