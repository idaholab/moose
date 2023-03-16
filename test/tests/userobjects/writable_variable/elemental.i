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
    family = MONOMIAL
    order = CONSTANT
  []
[]

[UserObjects]
  [elemental]
    type = MultiUpdateElementalUO
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
