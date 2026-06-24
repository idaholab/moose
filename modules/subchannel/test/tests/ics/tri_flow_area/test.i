[TriSubChannelMesh]
  [subchannel]
    type = SCMTriAssemblyMeshGenerator
    n_cells = 2
    nrings = 3
    flat_to_flat = 0.60
    heated_length = 1.0
    pin_diameter = 0.1
    pitch = 0.13
    dwire = 0.03
    hwire = 0.3
    spacer_k = '0.5'
    spacer_z = '0'
  []
[]

[Variables]
  [S]
    block = subchannel
  []
[]

[ICs]
  [S_IC]
    type = SCMTriFlowAreaIC
    variable = S
    block = subchannel
  []
[]

[Postprocessors]
  [center]
    type = SubChannelPointValue
    variable = S
    index = 0
    execute_on = 'timestep_end'
    height = 0.5
  []
  [edge]
    type = SubChannelPointValue
    variable = S
    index = 24
    execute_on = 'timestep_end'
    height = 0.5
  []
  [corner]
    type = SubChannelPointValue
    variable = S
    index = 25
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
