mass_flux_in = ${fparse 1e+6 * 17.00 / 3600.}

[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
    nx = 3
    ny = 3
    n_cells = 10
    n_blocks = 1
    pitch = 0.25
    rod_diameter = 0.125
    gap = 0.1
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
    type = QuadFlowAreaIC
    variable = S
  []

  [mdot_ic]
    type = MassFlowRateIC
    variable = mdot
    area = S
    mass_flux = ${mass_flux_in}
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
