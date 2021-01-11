mass_flux_in = ${fparse 1e+6 * 17.00 / 3600.}

[Mesh]
  type = QuadSubChannelMesh
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
