[QuadSubChannelMesh]
  [subchannel]
    type = SCMQuadAssemblyMeshGenerator
    nx = 6
    ny = 6
    n_cells = 3
    pitch = 0.0126
    pin_diameter = 0.00950
    side_gap = 0.00095
    heated_length = 1.0
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]


[AuxVariables]
  [S]
  []
  [mdot]
  []
[]

[ICs]
  [S_IC]
    type = ConstantIC
    variable = S
    value = 0.5
  []
[]

[AuxKernels]
  [mdot_ak]
    type = SCMBlockedMassFlowRateAux
    variable = mdot
    area = S
    index_blockage = '0'
    unblocked_mass_flux = 4722
    blocked_mass_flux = 1.0
    execute_on = 'initial'
  []
[]

[Postprocessors]
  [blocked_inlet]
    type = SubChannelPointValue
    variable = mdot
    index = 0
    height = 0.0
    execute_on = 'timestep_end'
  []
  [central_inlet]
    type = SubChannelPointValue
    variable = mdot
    index = 14
    height = 0.0
    execute_on = 'timestep_end'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
