[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
  []
[]

[AuxVariables]
  [v]
  []
[]

[UserObjects]
  [nodal1]
    type = MultiUpdateNodalUO
    v = v
    boundary = bottom
  []
  [nodal2]
    type = MultiUpdateNodalUO
    v = v
    boundary = right
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
