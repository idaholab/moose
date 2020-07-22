[Mesh]
  type = SubChannelMesh
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
  [q_prime]
  []
[]

[ICs]
  [q_prime_IC]
    type = PowerIC
    variable = q_prime
    power = 3.44e6 # W
    filename = "power_profile.txt" #type in name of file that describes power profile
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
