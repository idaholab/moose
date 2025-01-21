[GlobalParams]
  nx = 3
  ny = 3
  n_cells = 3
  pitch = 1
  heated_length = 0.2
  pin_diameter = 0.5
[]

[Mesh]
  [sub_channel]
    type = DetailedQuadSubChannelMeshGenerator
    gap = 0.1
  []

  [fuel_pins]
    type = DetailedQuadPinMeshGenerator
    input = sub_channel
  []
[]

[AuxVariables]
  [P]
    block = sub_channel
  []
  [T]
    block = fuel_pins
  []
[]

[Problem]
  type = NoSolveProblem
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
