[GlobalParams]
  nx = 3
  ny = 3
  n_cells = 3
  pitch = 1
  heated_length = 0.2
  pin_diameter = 0.5
[]

[Mesh]
  [assembly]
    type = SCMDetailedQuadAssemblyMeshGenerator
    side_gap = 0.1
  []
[]

[AuxVariables]
  [P]
    block = subchannel
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
