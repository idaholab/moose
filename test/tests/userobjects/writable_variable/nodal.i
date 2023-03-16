[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
  []
[]

[AuxVariables]
  [v]
    family = LAGRANGE
    order = FIRST
  []
[]

[UserObjects]
  [nodal]
    type = MultiUpdateNodalUO
    v = v
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
