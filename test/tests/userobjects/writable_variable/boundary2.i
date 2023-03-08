[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
    subdomain_ids = '1 2'
  []
[]

[AuxVariables]
  [v]
    block = 1
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
