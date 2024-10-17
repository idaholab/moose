[QuadSubChannelMesh]
  [subchannel]
    type = QuadSubChannelMeshGenerator
    nx = 6
    ny = 6
    n_cells = 3
    pitch = 0.0126
    pin_diameter = 0.00950
    gap = 0.00095 # the half gap between sub-channel assemblies
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
    type = BlockedMassFlowRateAux
    variable = mdot
    area = S
    index_blockage = '0'
    unblocked_mass_flux = 4722
    blocked_mass_flux = 1.0
    execute_on = 'initial'
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
