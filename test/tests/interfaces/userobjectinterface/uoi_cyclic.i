[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects]
  # 'A' references 'B' and 'B' references 'A': a genuine construction-time cycle that cannot be
  # resolved by reordering (idaholab/moose#7879).
  [A]
    type = UserObjectInterfaceTest
    uo = B
  []
  [B]
    type = UserObjectInterfaceTest
    uo = A
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
