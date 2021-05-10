
length = 1.0

[Mesh]
  type = TriSubChannelMesh
  generate_duct = false
  nrings = 3
  flat_to_flat = 0.60
  heated_length = ${length}
  rod_diameter = 0.1
  pitch = 0.13
  dwire = 0.03
  hwire = 0.3
  max_dz = 0.1
  spacer_k = '0.5'
  spacer_z = '0'
[]

[Variables]
  [q_prime]
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    value = '(pi/2)*sin(pi*z/L)'
    vars = 'L'
    vals = '${length}'
  []
[]

[ICs]
  [q_prime_IC]
    type = TriPowerIC
    variable = q_prime
    power = 5.0e5 # W
    filename = "pin_power_profile_axial.txt"
    axial_heat_rate = axial_heat_rate
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
