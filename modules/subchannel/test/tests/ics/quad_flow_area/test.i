[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadAssemblyMeshGenerator
    nx = 3
    ny = 3
    n_cells = 10
    pitch = 0.25
    pin_diameter = 0.125
    side_gap = 0.1
    heated_length = 1
    spacer_k = '0.0'
    spacer_z = '0'
  []
[]

[Variables]
  [S]
  []
[]

[ICs]
  [S_IC]
    type = SCMQuadFlowAreaIC
    variable = S
  []
[]

[Postprocessors]
  [center]
    type = SubChannelPointValue
    variable = S
    index = 4
    execute_on = 'timestep_end'
    height = 0.5
  []
  [edge]
    type = SubChannelPointValue
    variable = S
    index = 1
    execute_on = 'timestep_end'
    height = 0.5
  []
  [corner]
    type = SubChannelPointValue
    variable = S
    index = 0
    execute_on = 'timestep_end'
    height = 0.5
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = false
  csv = true
[]
