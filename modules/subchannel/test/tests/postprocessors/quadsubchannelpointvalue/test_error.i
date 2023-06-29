T_in = 359.15

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

[AuxVariables]
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

[Problem]
  type = NoSolveProblem
[]

[Postprocessors]
  [T]
    type = SubChannelPointValue
    variable = T
    index = 4
    execute_on = 'initial timestep_end'
    height = 5
  []
[]

[Outputs]
  csv = true
[]

[Executioner]
  type = Transient
  start_time = 0.0
  end_time = 10.0
  dt = 1.0
[]
