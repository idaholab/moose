[Mesh]
  type = QuadSubChannelMesh
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

[Variables]
  [q_prime]
  []
[]

[ICs]
  [q_prime_IC]
    type = QuadPowerIC
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
