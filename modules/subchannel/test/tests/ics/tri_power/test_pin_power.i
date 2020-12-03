[Mesh]
  type = TriSubChannelMesh
  nrings = 3
  flat_to_flat = 0.60
  heated_length = 1.0
  rod_diameter = 0.1
  pitch = 0.13
  max_dz = 0.1
  spacer_k = '0.5'
  spacer_z = '0'
[]

[Variables]
  [q_prime]
  []
[]

[ICs]
  [q_prime_IC]
    type = TriPowerIC
    variable = q_prime
    power = 5.0e5 # W
    filename = "pin_power_profile.txt"
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
