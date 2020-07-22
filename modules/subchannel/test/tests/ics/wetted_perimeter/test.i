[Mesh]
  type = SubChannelMesh
  nx = 3
  ny = 3
  max_dz = 2.
  pitch = 1.26
  rod_diameter = 0.950
  gap = 0.095
  heated_length = 1
  spacer_z = '0.5'
  spacer_k = '0.1'
[]

[Variables]
  [w_perim]
  []
[]

[ICs]
  [w_perim_IC]
    type = WettedPerimIC
    variable = w_perim
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
