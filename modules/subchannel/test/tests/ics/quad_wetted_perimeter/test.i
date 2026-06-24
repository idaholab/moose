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
  [w_perim]
  []
[]

[ICs]
  [w_perim_IC]
    type = SCMQuadWettedPerimIC
    variable = w_perim
  []
[]

[Postprocessors]
  [center]
    type = SubChannelPointValue
    variable = w_perim
    index = 4
    execute_on = 'timestep_end'
    height = 0.5
  []
  [edge]
    type = SubChannelPointValue
    variable = w_perim
    index = 1
    execute_on = 'timestep_end'
    height = 0.5
  []
  [corner]
    type = SubChannelPointValue
    variable = w_perim
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
