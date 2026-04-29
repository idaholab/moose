################################################################################
## MARVEL-type SCM geometry setup simulation                                  ##
## POC : Vasileios Kyriakopoulos, vasileios.kyriakopoulos@inl.gov             ##
################################################################################
heated_length = 0.51
[TriSubChannelMesh]
  [subchannel]
    type = SCMTriAssemblyMeshGenerator
    nrings = 4
    n_cells = 40
    flat_to_flat = 0.22
    heated_length = ${heated_length}
    unheated_length_entry = 0.2
    unheated_length_exit = 0.2
    pin_diameter = 0.03269
    pitch = 0.0346514
    dwire = 0.0
    hwire = 0.0
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]

[AuxVariables]
  [S]
    block = subchannel
  []
  [w_perim]
    block = subchannel
  []
[]


[Problem]
    type = NoSolveProblem
[]

[ICs]
  [S_IC]
    type = MarvelTriFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = MarvelTriWettedPerimIC
    variable = w_perim
  []
[]

[Postprocessors]
  [center_S]
    type = SubChannelPointValue
    variable = S
    index = 0
    execute_on = 'timestep_end'
    height = 0.5
  []
  [edge_S]
    type = SubChannelPointValue
    variable = S
    index = 54
    execute_on = 'timestep_end'
    height = 0.5
  []
  [corner_S]
    type = SubChannelPointValue
    variable = S
    index = 55
    execute_on = 'timestep_end'
    height = 0.5
  []
  [center_w_perim]
    type = SubChannelPointValue
    variable = w_perim
    index = 0
    execute_on = 'timestep_end'
    height = 0.5
  []
  [edge_w_perim]
    type = SubChannelPointValue
    variable = w_perim
    index = 54
    execute_on = 'timestep_end'
    height = 0.5
  []
  [corner_w_perim]
    type = SubChannelPointValue
    variable = w_perim
    index = 55
    execute_on = 'timestep_end'
    height = 0.5
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = false
  csv = true
[]
