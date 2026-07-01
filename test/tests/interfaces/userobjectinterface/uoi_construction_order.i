[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects]
  # 'test' references 'other_uo' in its constructor but is declared first. The framework must
  # construct 'other_uo' before 'test' regardless of input order (idaholab/moose#7879).
  [test]
    type = UserObjectInterfaceTest
    uo = other_uo
    has = true
  []
  [other_uo]
    type = UserObjectInterfaceTest
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
