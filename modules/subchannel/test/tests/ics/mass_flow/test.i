mass_flux_in = ${fparse 1e+6 * 17.00 / 3600.}

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

  [mdot]
  []
[]

[ICs]
  [S_IC]
    type = SCMQuadFlowAreaIC
    variable = S
  []

  [mdot_ic]
    type = SCMMassFlowRateIC
    variable = mdot
    area = S
    mass_flux = ${mass_flux_in}
  []
[]

[Postprocessors]
  [center]
    type = SubChannelPointValue
    variable = mdot
    index = 4
    execute_on = 'timestep_end'
    height = 0.5
  []
  [edge]
    type = SubChannelPointValue
    variable = mdot
    index = 1
    execute_on = 'timestep_end'
    height = 0.5
  []
  [corner]
    type = SubChannelPointValue
    variable = mdot
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
